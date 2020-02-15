import { api as Sodium } from 'sodium';

import { UInt53toBufferLE } from './number';

const debug = require('debug')('encryption');

function computePoly1305(cipherText, AAD, nonce, key) {
    if (AAD == null) {
        AAD = Buffer.alloc(0);
    }

    const msg =
        Buffer.concat([
            AAD,
            getPadding(AAD, 16),
            cipherText,
            getPadding(cipherText, 16),
            UInt53toBufferLE(AAD.length),
            UInt53toBufferLE(cipherText.length)
        ])

    const polyKey = Sodium.crypto_stream_chacha20(32, nonce, key);
    const computed_hmac = Sodium.crypto_onetimeauth(msg, polyKey);
    polyKey.fill(0);

    return computed_hmac;
}

// i'd really prefer for this to be a direct call to
// Sodium.crypto_aead_chacha20poly1305_decrypt()
// but unfortunately the way it constructs the message to
// calculate the HMAC is not compatible with homekit
// (long story short, it uses [ AAD, AAD.length, CipherText, CipherText.length ]
// whereas homekit expects [ AAD, CipherText, AAD.length, CipherText.length ]
function verifyAndDecrypt(cipherText, mac, AAD, nonce, key) {
    const matches =
        Sodium.crypto_verify_16(
            mac,
            computePoly1305(cipherText, AAD, nonce, key)
        );

    if (matches === 0) {
        return Sodium
            .crypto_stream_chacha20_xor_ic(cipherText, nonce, 1, key);
    }

    return null;
}

// See above about calling directly into libsodium.
function encryptAndSeal(plainText, AAD, nonce, key) {
    const cipherText =
        Sodium
            .crypto_stream_chacha20_xor_ic(plainText, nonce, 1, key);

    const hmac =
        computePoly1305(cipherText, AAD, nonce, key);

    return [ cipherText, hmac ];
}

function getPadding(buffer, blockSize) {
    return buffer.length % blockSize === 0
        ? Buffer.alloc(0)
        : Buffer.alloc(blockSize - (buffer.length % blockSize))
}

export default {
    encryptAndSeal,
    verifyAndDecrypt
}
