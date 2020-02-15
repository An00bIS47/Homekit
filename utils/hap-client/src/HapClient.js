import Enum from 'enum';
import { Observable, Subject } from 'rxjs';

import srp from 'fast-srp-hap';
import HKDF from 'node-hkdf-sync';
import { api as Sodium } from 'sodium';
import uuidv5 from 'uuidv5';

import EventedHttpClient from './EventedHttpClient';
import tlv from './lib/tlv.js';
import enc from './lib/encryption';
import Cryptographer from './lib/cryptographer';
import AuthHeader from './lib/authorization';
import SecureStore from './SecureStore';

const debug = require('debug')('hap-client:hap');

const { Tag } = tlv;

const ErrorCode =
    new Enum({
        None:                      0x00,
        Unknown:                   0x01,
        AuthenticationFailed:      0x02, // eg client proof is wrong
        TooManyAttempts:           0x03,
        UnknownPeer:               0x04,
        MaxPeer:                   0x05,
        MaxAuthenticationAttempts: 0x06
    });

const PairStep = 
    new Enum({
        StartRequest:        0x01,
        StartResponse:       0x02, 
        VerifyRequest:       0x03,
        VerifyResponse:      0x04,
        KeyExchangeRequest:  0x05,
        KeyExchangeResponse: 0x06
    });

const VerifyStep =
    new Enum({
        StartRequest:        0x01,
        StartResponse:       0x02, 
        FinishRequest:       0x03,
        FinishResponse:      0x04
    });

function getSession(secureStore, seed) {
    return secureStore
        .getClient()
        .flatMap(
            clientInfo => {
                let saveClientInfo =
                    Observable
                        .empty();

                if (!clientInfo.longTerm) {
                    debug('Generating long-term keys');
                    clientInfo.longTerm =
                        Sodium.crypto_sign_keypair();

                     saveClientInfo =
                        Observable
                            .from(
                                clientInfo.save()
                            )
                            .ignoreElements();
                }

                debug('Reusing long-term keys');
                return saveClientInfo
                    .concat(
                        Observable
                            .of({ clientInfo })
                    );
            }
        )
        .map(
            data => ({
                ...seed,
                ...data
            })
        )
}

function getAt(idx) {
    return this && this[idx];
}

function handleResponse(vtable, steps, session, completeCondition = () => false) {
    return function(response) {
        if (response && !completeCondition(response)) {
            if (response.status < 200 || response.status >= 400) {
                return Observable.throw(new Error(`Bad status: ${response.status} ${response.statusText}`));
            }

            const data = response.body;

            const errorCode = data[Tag.ErrorCode]::getAt(0);
            if (errorCode) {
                return Observable.throw(new Error("Pairing failure: " + ErrorCode.get(errorCode).key));
            }
            
            const step = steps.get(data[Tag.Sequence][0]);

            debug("--> [%d] %o", step, data)

            let handler = vtable[step];
            return handler
                ? handler(response, data, session)
                : Observable.empty();
        }
        return Observable.empty();
    }
}

function toNodeObservable () {
    var self = this;
    return function() {
        return Observable.bindNodeCallback(self)(...arguments)
    }
}

const PairingContentType = 'application/pairing+tlv8';

const CLIENT_ID_NAMESPACE = 'fdde9099-dae4-4a18-ad5d-a2b07b8ebb9b';

class HapClient
{
    constructor(clientName, ip, port) {
        Object.defineProperty(
            this, '_client', {
                value: new EventedHttpClient(ip, port)
            }
        );

        Object.defineProperty(
            this, '_clientId', {
                value: uuidv5(CLIENT_ID_NAMESPACE, clientName)
            }
        );

        Object.defineProperty(
            this, '_clientName', {
                value: clientName
            }
        );
    }

    _pair(pinProvider) {
        const clientId = this._clientId;
        const secureStore = new SecureStore(this._clientName);

        return getSession(secureStore, { http: this._client })
            .flatMap(
                session => {
                    // steps:
                    //   1. POST the pairing request to /pair-setup
                    const req =
                        tlv.encode(
                            Tag.PairingMethod, 0,
                            Tag.Sequence, PairStep.StartRequest.value
                        );

                    debug("encoded request: %o", req);

                    return session.http.post('/pair-setup', req, PairingContentType)
                        .expand(
                            handleResponse(
                                {
                                    [PairStep.StartResponse]:       handlePairStartResponse,
                                    [PairStep.VerifyResponse]:      handlePairVerifyResponse,
                                    [PairStep.KeyExchangeResponse]: handlePairKeyExchangeResponse
                                },
                                PairStep,
                                session,
                                // complete condition
                                x => x.finished
                            )
                        )
                        .takeLast(1)
                }
            );

        function handlePairStartResponse(response, data, session) {
            //   2. Read the server's salt and public key
            const salt = data[Tag.Salt], serverPublicKey = data[Tag.PublicKey];

            // 2a. validate
            if (salt.length != 16) {
                return Observable.throw(new Error("salt must be 16 bytes"));
            }
            if (serverPublicKey.length != 384) {
                return Observable.throw(new Error(`serverPublicKey must be 384 bytes (but was ${serverPublicKey.length})`));
            }

            debug(" -> s: %s", salt.toString('hex'));
            debug(" -> B: %s", serverPublicKey.toString('hex'));

            const genKey = ::srp.genKey::toNodeObservable()
            return genKey()
                .flatMap(
                    a =>
                        Observable
                            .from(
                                pinProvider
                            )
                            .map(pin => {
                                debug("got code: >%s<", pin);
                                session.pinCode = pin;
                                return a;
                            })
                )
                .flatMap(
                    a => {
                        debug("a: " + a.toString('hex'));
                        //   3. Generate my key pair <-- requires knowing PIN
                        session.rp =
                            new srp.Client(
                                srp.params['3072'],
                                salt,
                                Buffer.from('Pair-Setup'),
                                Buffer.from(session.pinCode),
                                a
                            );
                        session.rp.setB(serverPublicKey);

                        //   4. POST my public key and password proof to /pair-setup
                        const A = session.rp.computeA(),
                             M1 = session.rp.computeM1()
                            ;

                        debug(" <-  A: %s", A.toString('hex'));
                        debug(" <- M1: %s", M1.toString('hex'));

                        const verifyRequest =
                            tlv.encode(
                                Tag.PairingMethod, 0,
                                Tag.Sequence, PairStep.VerifyRequest.value,
                                Tag.PublicKey, A,
                                Tag.Proof, M1
                            );
                        debug("encoded request: %o", verifyRequest);

                        return session.http
                            .post('/pair-setup', verifyRequest, PairingContentType);
                    }
                )
        }

        function handlePairVerifyResponse(response, data, session) {
            debug('got a verify response');

            //   5. Read and verify the server's password proof
            const serverProof = data[Tag.Proof];
            debug(" -> M2: %s", serverProof.toString('hex'));

            try {
                session.rp.checkM2(serverProof);
            } catch (e) {
                return Observable.throw(new Error("Server proof is invalid: " + e));
            }

            //   6. Generate encryption key
            session.encryptionKey =
                new HKDF(
                    'sha512',
                    'Pair-Setup-Encrypt-Salt',
                    session.rp.computeK()
                ).derive('Pair-Setup-Encrypt-Info', 32);

            debug("key: %s", session.encryptionKey.toString('hex'));

            //   7. POST an encrypted message to /pair-setup
            const hash =
                new HKDF(
                    'sha512',
                    'Pair-Setup-Controller-Sign-Salt',
                    session.rp.computeK()
                ).derive('Pair-Setup-Controller-Sign-Info', 32);

            const material =
                Buffer
                    .concat([
                        hash,
                        Buffer.from(clientId),
                        session.clientInfo.longTerm.publicKey
                    ]);

            const signature =
                Sodium
                    .crypto_sign_detached(
                        material,
                        session.clientInfo.longTerm.secretKey);

            const message =
                tlv.encode(
                    Tag.Username, clientId,
                    Tag.PublicKey, session.clientInfo.longTerm.publicKey,
                    Tag.Signature, signature
                );

            const [ encrypted, seal ] =
                enc.encryptAndSeal(
                    message,
                    null,
                    Buffer.from('PS-Msg05'),
                    session.encryptionKey
                );

            debug('encrypted: ' + encrypted.toString('hex'));
            debug('seal: ' + seal.toString('hex'));

            const container =
                tlv.encode(
                    Tag.PairingMethod, 0x0,
                    Tag.Sequence, PairStep.KeyExchangeRequest.value,
                    Tag.EncryptedData, Buffer.concat([ encrypted, seal ])
                );

            return session.http
                .post('/pair-setup', container, PairingContentType);
        }
        
        function handlePairKeyExchangeResponse(response, data, session) {
            debug('got a key exchange response');

            //   8. Read encrypted response; decode to reveal username, long-term public key, signature
            const ciphertext = data[Tag.EncryptedData];
            const [ encrypted, hmac ] = [ ciphertext.slice(0, -16), ciphertext.slice(-16) ];

            debug('message: ' + encrypted.toString('hex'));
            debug('hmac: ' + hmac.toString('hex'));

            const message =
                enc.verifyAndDecrypt(
                    encrypted,
                    hmac,
                    null,
                    Buffer.from("PS-Msg06"),
                    session.encryptionKey
                );

            if (message) {
                const data = tlv.decode(message);
                const accName = data[Tag.Username];
                const accLTPK = data[Tag.PublicKey];
                const accSign = data[Tag.Signature];

                debug('accessory name: ' + accName.toString('utf8'));
                debug('LTPK: ' + accLTPK.toString('hex'));

                const hash =
                    new HKDF(
                        'sha512',
                        'Pair-Setup-Accessory-Sign-Salt',
                        session.rp.computeK()
                    ).derive('Pair-Setup-Accessory-Sign-Info', 32);

                const material = Buffer.concat([ hash, accName, accLTPK ]);
                if (Sodium.crypto_sign_verify_detached(accSign, material, accLTPK)) {
                    //   WE ARE NOW PAIRED.

                    return secureStore
                        .get(accName.toString('utf8'))
                        .flatMap(
                            accessoryInfo => {
                                accessoryInfo.pin = session.pinCode;
                                accessoryInfo.ltpk = accLTPK;

                                return accessoryInfo.save();
                            }
                        )
                        .ignoreElements()
                        .concat(
                            Observable.of({
                                finished: true,
                                session
                            })
                        )
                    ;
                }

                return Observable.throw(new Error("Unable to verify key exchange; PAIRING FAILED."));
            }

            return Observable.throw(new Error("Unable to decrypt key exchange; PAIRING FAILED."));
        }
    }

    _verifyPairing() {
        const clientId = this._clientId;
        const secureStore = new SecureStore(this._clientName);

        // generate new encryption keys for this session
        const privateKey = Buffer.alloc(32);
        Sodium.randombytes_buf(privateKey);

        const publicKey =
            Sodium.crypto_scalarmult_base(privateKey);

        return Observable
            .defer(
                () =>
                    getSession(
                        secureStore,
                        {
                            privateKey,
                            publicKey,
                            http: this._client
                        })
            )
            .flatMap(
                session => {
                    const req = tlv.encode(
                        Tag.PairingMethod, 0,
                        Tag.Sequence, VerifyStep.StartRequest.value,
                        Tag.PublicKey, session.publicKey
                    );
                    debug("encoded request: %o", req);

                    //   1. POST my public key (etc) to /pair-verify
                    return session.http.post('/pair-verify', req, PairingContentType)
                        .expand(
                            handleResponse(
                                {
                                    [VerifyStep.StartResponse]:  handleVerifyStartResponse,
                                    [VerifyStep.FinishResponse]: handleVerifyFinishResponse
                                },
                                VerifyStep,
                                session,
                                x => x.finished
                            )
                        )
                        .takeLast(1);
                }
            )
            ;

        function handleVerifyStartResponse(response, data, session) {
            //   2. Read the server's public key
            const serverPublicKey = session.serverPublicKey = data[Tag.PublicKey];

            // 2a. validate
            if (serverPublicKey.length != 32) {
                return Observable.throw(new Error(`serverPublicKey must be 32 bytes (but was ${serverPublicKey.length})`));
            }

            //   3. Convert to shared key
            session.sharedKey =
                Sodium
                    .crypto_scalarmult(
                        session.privateKey,
                        serverPublicKey
                    );
                
            session.encryptionKey = 
                new HKDF(
                    'sha512',
                    'Pair-Verify-Encrypt-Salt',
                    session.sharedKey
                ).derive('Pair-Verify-Encrypt-Info', 32);

            //   4. Decrypt and validate the message
            const ciphertext = data[Tag.EncryptedData];
            const [ encrypted, hmac ] = [ ciphertext.slice(0, -16), ciphertext.slice(-16) ];

            const message =
                enc.verifyAndDecrypt(
                    encrypted,
                    hmac,
                    null,
                    Buffer.from("PV-Msg02"),
                    session.encryptionKey
                );

            if (message) {
                const data = tlv.decode(message);
                const username = data[Tag.Username];
                const signature = data[Tag.Signature];

                const user = username.toString('utf8');
                debug("got username: " + username.toString('utf8'));
                return secureStore
                    .get(user)
                    .flatMap(
                        accessoryInfo => {
                            session.accessoryInfo = accessoryInfo;

                            const material =
                                Buffer.concat([ session.serverPublicKey, username, session.publicKey ]);
                            return !accessoryInfo.ltpk
                                ? Observable.throw(`Could not get LTPK for accessory ${user}`)
                                : Sodium.crypto_sign_verify_detached(signature, material, accessoryInfo.ltpk)
                                    ? Observable.of(accessoryInfo)
                                    : Observable.throw(`Could not verify signature for accessory ${user}`)
                        }
                    )
                    .map(
                        accessoryInfo => {
                            //   5. Generate a response message
                            const material =
                                Buffer
                                    .concat([
                                        session.publicKey,
                                        Buffer.from(clientId),
                                        session.serverPublicKey
                                    ]);

                            const plaintext =
                                tlv.encode(
                                    Tag.Username, clientId,
                                    Tag.Signature, 
                                        Sodium
                                            .crypto_sign_detached(
                                                material,
                                                session.clientInfo.longTerm.secretKey)
                                );

                            const [ encrypted, seal ] =
                                enc.encryptAndSeal(
                                    plaintext,
                                    null,
                                    Buffer.from('PV-Msg03'),
                                    session.encryptionKey
                                );

                            return tlv.encode(
                                Tag.PairingMethod, 0,
                                Tag.Sequence, VerifyStep.FinishRequest.value,
                                Tag.EncryptedData, Buffer.concat([ encrypted, seal ])
                            );
                        }
                    )
                    .flatMap(
                        out =>
                            //   6. POST it to /pair-verify
                            session.http.post('/pair-verify', out, PairingContentType)
                    )
            }

            return Observable.throw(new Error("Unable to decrypt verification info; VERIFICATION FAILED."));
        }

        function handleVerifyFinishResponse(response, data, session) {
            // if we get here, there wasn't an error (because handleResponse()
            // checks for us).
            //
            //   WE ARE NOW VERIFIED.

            debug('Verification complete.');

            var encSalt = Buffer.from("Control-Salt");
            var infoRead = Buffer.from("Control-Read-Encryption-Key");
            var infoWrite = Buffer.from("Control-Write-Encryption-Key");

            return Observable.of({
                finished: true,
                accessoryInfo: session.accessoryInfo,
                cryptographer:
                    new Cryptographer(
                        new HKDF(
                            'sha512',
                            encSalt,
                            session.sharedKey
                        ).derive(infoWrite, 32),
                        new HKDF(
                            'sha512',
                            encSalt,
                            session.sharedKey
                        ).derive(infoRead, 32)
                    )
            });
        }
    }

    _ensureAuthenticated() {
        // TODO: is there a better way than tracking this bool?
        if (this._isAuthenticated) {
            return Observable.of(this._client);
        }
        else if (this._authPending) {
            let s = new Subject();
            this._authPending.push(s);
            return s.asObservable();
        }
        else {
            this._authPending = [];
            return this
                ._verifyPairing()
                .map(({ finished, ...session }) => {
                    this._client.addMiddleware(session.cryptographer);
                    this._client.addMiddleware(new AuthHeader(session.accessoryInfo.pin));
                    this._isAuthenticated = true;
                    return this._client;
                })
                .do(
                    c => {
                        this
                            ._authPending
                            .forEach(
                                s => s.next(c)
                            )
                        delete this._authPending;
                    }
                );
        }
    }

    get messages() {
        return this
            ._ensureAuthenticated()
            .flatMap(
                client => client.messages
            )
    }

    pair(pinProvider) {
        return this
            ._pair(pinProvider)
            .ignoreElements();
    }

    verifyPairing() {
        return this
            ._verifyPairing()
            .ignoreElements();
    }
    
    identify() {
        return this._client.get('/identify');
    }

    listAccessories() {
        return this
            ._ensureAuthenticated()
            .flatMap(
                client =>
                    client
                        .get('/accessories')
                        .map(
                            ({ status, body }) =>
                                (status >= 200 && status < 400)
                                    ? body
                                    : Observable.throw(new Error(statusText))
                        )
            )
    }

    getCharacteristics(...args /* aid, iid, aid, iid ...  */) {
        if (args.length % 2 !== 0) {
            throw new TypeError("getCharacteristics must be given an even number of arguments");
        }

        debug("building query for %o", args);

        return Observable
            .from(args)
            .bufferCount(2)
            .map(([ aid, iid ]) => `${aid}.${iid}`)
            .toArray()
            .map(x => x.join(','))
            .flatMap(
                query =>
                    this
                        ._ensureAuthenticated()
                        .flatMap(
                            client =>
                                client
                                    .get(`/characteristics?id=${query}`)
                        )
                        .map(
                            ({ status, body }) =>
                                (status >= 200 && status < 400)
                                    ? body
                                    : Observable.throw(new Error(statusText))
                        )
            )
    }

    setCharacteristics(...characteristics) {
        return this
            ._ensureAuthenticated()
            .flatMap(
                client =>
                    client
                        .put(
                            '/characteristics',
                            { characteristics },
                            'application/hap+json'
                        )
            )
            .map(
                ({ status, body }) =>
                    (status >= 200 && status < 400)
                        ? body
                        : Observable.throw(new Error(statusText))
            )
    }

    close() {
        this._client.disconnect();
    }
}

export {
    HapClient as default
}
