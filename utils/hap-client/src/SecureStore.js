import { Observable } from 'rxjs';
import Keytar from 'keytar';

const debug = require('debug')('hap-client:securestore');

function loadClient(clientName) {
    return Observable
        .defer(
            () =>
                Keytar
                    .getPassword(clientName, 'clientInfo')
        )
        .map(
            json =>
                new SecureClientInfo(clientName, json)
        )
}

function saveClient(self) {
    return Keytar
        .setPassword(
            self._clientName,
            'clientInfo',
            JSON.stringify(self)
        )
    .then(() => self);
}

function load(clientName, username) {
    debug(`loading for ${clientName}/${username}`);
    return Observable
        .from(
            Keytar
                .getPassword(clientName, username)
        )
        .map(
            json =>
                new SecureAccessoryInfo(clientName, username, json)
        )
        ;
}

function save(self) {
    debug(`saving for ${self._clientName}/${self.user}`);
    debug(`${JSON.stringify(self)}`);
    return Keytar
        .setPassword(
            self._clientName,
            self.user,
            JSON.stringify(self)
        )
        .then(() => self);
}

class SecureAccessoryInfo
{
    constructor(clientName, username, json) {
        this._username = username;
        this._clientName = clientName;

        if (json) {
            var data = JSON.parse(json);
            this._ltpk = Buffer.from(data.ltpk, 'base64');
            this._pin = data.pin;
        } else {
            this._pin = this._ltpk = '';
        }
    }

    toJSON() {
        return {
            ltpk: this._ltpk.toString('base64'),
            pin: this._pin
        };
    }

    get user() {
        return this._username;
    }

    get ltpk() {
        return this._ltpk;
    }

    set ltpk(value) {
        this._ltpk = value;
    }

    get pin() {
        return this._pin;
    }

    set pin(value) {
        this._pin = value;
    }

    save() {
        return save(this);
    }
}

class SecureClientInfo
{
    constructor(clientName, json) {
        this._clientName = clientName;

        if (json) {
            var data = JSON.parse(json);
            this._longTerm = {
                secretKey: Buffer.from(data.longTerm.secretKey, 'base64'),
                publicKey: Buffer.from(data.longTerm.publicKey, 'base64')
            };
        } else {
            this._longTerm = '';
        }
    }

    toJSON() {
        return {
            longTerm: {
                publicKey: this._longTerm.publicKey.toString('base64'),
                secretKey: this._longTerm.secretKey.toString('base64')
            }
        };
    }

    get longTerm() {
        return this._longTerm;
    }

    set longTerm(value) {
        this._longTerm = value;
    }

    save() {
        return saveClient(this);
    }
}

class SecureStore {
    constructor(clientName) {
        this._clientName = clientName;
    }
    get(username) {
        return load(this._clientName, username)
    }
    getClient() {
        return loadClient(this._clientName)
    }
}

export {
    SecureStore as default
}
