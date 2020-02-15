var assert = require('assert');

const tlv = require('../dist/lib/tlv.js').default;
//const tlv = require('/home/zb/stuff/hap-server/HAP-NodeJS/lib/util/tlv.js');


console.dir(tlv);

describe('tlv', function() {
    describe('#encode()', function() {
        it('should encode single fields of 1 byte', function() {
            const enc = tlv.encode(0x01, 0x7f);
            assert(enc.equals(new Buffer([ 0x01, 0x01, 0x7f ])));
        });

        it('should encode multiple fields of 1 byte', function() {
            const enc = tlv.encode(0x01, 0x7f, 0x02, 0x91);
            assert(enc.equals(new Buffer([ 0x01, 0x01, 0x7f, 0x02, 0x01, 0x91 ])));
        });

        it('should encode single fields of several bytes', function() {
            const enc = tlv.encode(0x01, Buffer.from('foobar', 'utf8'));
            assert(enc.equals(new Buffer([ 0x01, 0x06, 0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72 ])));
        });

        it('should encode multiple fields of several bytes', function() {
            const enc = tlv.encode(0x01, Buffer.from('foobar', 'utf8'), 0x02, Buffer.from('bazquux', 'utf8'));
            assert(
                enc.equals(
                    new Buffer([
                        0x01, 0x06, 0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72,
                        0x02, 0x07, 0x62, 0x61, 0x7A, 0x71, 0x75, 0x75, 0x78 
                    ])
                )
            );
        });

        it('should encode single fields of loads of bytes', function() {
            const string = " ".repeat(384);
            const enc = tlv.encode(0x01, Buffer.from(string, 'utf8'));

            assert(
                enc.equals(
                    Buffer
                        .concat([
                            new Buffer([ 0x01, 0xFF ]), Buffer.from(" ".repeat(255)),
                            new Buffer([ 0x01, 0x81 ]), Buffer.from(" ".repeat(129))
                        ])
                )
            );
        });

        it('should encode multiple fields of loads of bytes', function() {
            const string1 = " ".repeat(384);
            const string2 = "x".repeat(384);
            const enc =
                tlv.encode(
                    0x01, Buffer.from(string1, 'utf8'),
                    0x02, Buffer.from(string2, 'utf8')
                );

            assert(
                enc.equals(
                    Buffer
                        .concat([
                            new Buffer([ 0x01, 0xFF ]), Buffer.from(" ".repeat(255)),
                            new Buffer([ 0x01, 0x81 ]), Buffer.from(" ".repeat(129)),
                            new Buffer([ 0x02, 0xFF ]), Buffer.from("x".repeat(255)),
                            new Buffer([ 0x02, 0x81 ]), Buffer.from("x".repeat(129))
                        ])
                )
            );
        });
    });

    describe('#decode()', function() {
        function deepCompare(obj1, obj2) {
            // the set of keys must match
            assert(
                Object
                    .keys(obj1)
                    .every(k => obj2.hasOwnProperty(k))
            );

            // for each key, the values must be the same
            Object
                .keys(obj1)
                .forEach(
                    k => {
                        assert.equal(typeof obj1[k], typeof obj2[k]);

                        if (typeof obj1[k] === 'object') {
                            assert.equal(obj1[k].constructor, obj2[k].constructor);

                            if (obj1[k].constructor != Buffer) {
                                deepCompare(obj1[k], obj2[k]);
                            }
                            else {
                                assert(obj1[k].equals(obj2[k]));
                            }
                        }
                        else {
                            assert.equal(obj1[k], obj2[k]);
                        }
                    }
                );
        }

        it('should decode single fields of 1 byte', function() {
            const test = new Buffer([ 0x01, 0x01, 0x7f ]);

            deepCompare(
                {
                    [0x01]: new Buffer([ 0x7f ])
                },
                tlv.decode(test)
            );
        });

        it('should decode multiple fields of 1 byte', function() {
            const test = new Buffer([ 0x01, 0x01, 0x7f, 0x02, 0x01, 0x91 ]);

            deepCompare(
                {
                    [0x01]: new Buffer([ 0x7f ]),
                    [0x02]: new Buffer([ 0x91 ]),
                },
                tlv.decode(test)
            );
        });

        it('should decode single fields of several bytes', function() {
            const test = new Buffer([ 0x01, 0x06, 0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72 ]);

            deepCompare(
                {
                    [0x01]: Buffer.from('foobar', 'utf8')
                },
                tlv.decode(test)
            );
        });

        it('should decode multiple fields of several bytes', function() {
            const test =
                new Buffer([
                    0x01, 0x06, 0x66, 0x6F, 0x6F, 0x62, 0x61, 0x72,
                    0x02, 0x07, 0x62, 0x61, 0x7A, 0x71, 0x75, 0x75, 0x78 
                ]);

            deepCompare(
                {
                    [0x01]: Buffer.from('foobar', 'utf8'),
                    [0x02]: Buffer.from('bazquux', 'utf8')
                },
                tlv.decode(test)
            );
        });

        it('should decode single fields of loads of bytes', function() {
            const test = 
                Buffer
                    .concat([
                        new Buffer([ 0x01, 0xFF ]), Buffer.from(" ".repeat(255)),
                        new Buffer([ 0x01, 0x81 ]), Buffer.from(" ".repeat(129))
                    ]);

            deepCompare(
                {
                    [0x01]: Buffer.from(" ".repeat(384), 'utf8')
                },
                tlv.decode(test)
            );
        });

        it('should decode multiple fields of loads of bytes', function() {
            const test = 
                Buffer
                    .concat([
                        new Buffer([ 0x01, 0xFF ]), Buffer.from(" ".repeat(255)),
                        new Buffer([ 0x01, 0x81 ]), Buffer.from(" ".repeat(129)),
                        new Buffer([ 0x02, 0xFF ]), Buffer.from("x".repeat(255)),
                        new Buffer([ 0x02, 0x81 ]), Buffer.from("x".repeat(129))
                    ]);

            deepCompare(
                {
                    [0x01]: Buffer.from(" ".repeat(384), 'utf8'),
                    [0x02]: Buffer.from("x".repeat(384), 'utf8')
                },
                tlv.decode(test)
            );
        });
    });
});
