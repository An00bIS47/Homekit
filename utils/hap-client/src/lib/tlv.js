const debug = require('debug')('hap-client:tlv');

/**
 * Type Length Value encoding/decoding, used by HAP as a wire format.
 * https://en.wikipedia.org/wiki/Type-length-value
 *
 * Originally based on code from github:KhaosT/HAP-NodeJS@0c8fd88 used
 * used per the terms of the Apache Software License v2.
 *
 * Original code copyright Khaos Tian <khaos.tian@gmail.com>
 *
 * Modifications copyright Zach Bean <zb@forty2.com>
 *  * Reformatted for ES6-style module
 *  * Rewrote encode() to be non-recursive; also simplified the logic
 *  * Rewrote decode()
 */

const Tag = {
    PairingMethod:  0x00,
	Username:       0x01,
	Salt:           0x02, // salt is 16 bytes long

    // could be either the SRP client public key (384 bytes) or the ED25519 public key (32 bytes), depending on context
	PublicKey:      0x03,
	Proof:          0x04, // 64 bytes
	EncryptedData:  0x05,
	Sequence:       0x06,
	ErrorCode:      0x07,
	Signature:      0x0A, // 64 bytes

	MFiCertificate: 0x09,
	MFiSignature:   0x0A
};

function append(...buffers) {
    return Buffer.concat([ this, ...buffers ]);
}

function encode(...args /* type, data, type, data... */) {
    if (args.length % 2 !== 0) {
        throw new TypeError("encode must be given an even number of arguments");
    }

    var encodedTLVBuffer = Buffer.alloc(0);

    let type, data;
    while (([ type, data, ...args ] = args) && (typeof type !== 'undefined')) {
        // coerce data to Buffer if needed
        if (typeof data === 'number') {
            debug("turning %d into buffer", data);
            data = Buffer.from([ data ]);
        }
        else if (typeof data === 'string') {
            debug("turning %s into buffer", data);
            data = Buffer.from(data);
        }

        // break into chunks of at most 255 bytes
        let pos = 0;
        while (data.length - pos > 0) {
            let len = Math.min(data.length - pos, 255);

            debug(`adding ${len} bytes of type ${type} to the buffer starting at ${pos}`);

            encodedTLVBuffer =
                encodedTLVBuffer
                    ::append(
                        Buffer.from([ type, len ]),
                        data.slice(pos, pos + len)
                    );

            pos += len;
        }
    }

    return encodedTLVBuffer;
}

function decode(data) {
    let pos = 0, ret = {};
    while (data.length - pos > 0) {
        let [ type, length ] = data.slice(pos);

        pos += 2;

        let newData = data.slice(pos, pos + length);

        if (ret[type]) {
            ret[type] = ret[type]::append(newData);
        } else {
            ret[type] = newData;
        }

        pos += length;
    }

    return ret;
}

export default {
    Tag,
    encode,
    decode
}
