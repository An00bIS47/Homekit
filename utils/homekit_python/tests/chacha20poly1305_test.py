#
# Copyright 2018 Joachim Lusiardi
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import unittest

from homekit.crypto.chacha20poly1305 import pad16, chacha20_quarter_round, chacha20_create_initial_state, \
    chacha20_aead_decrypt, chacha20_aead_verify_tag, chacha20_aead_encrypt, chacha20_block, chacha20_encrypt, calc_s, \
    calc_r, clamp, poly1305_key_gen, poly1305_mac


class TestChacha20poly1305(unittest.TestCase):

    def test_pad16_does_not_pad_multiples_of_16(self):
        input_data = b'1234567890ABCDEF'
        pad = pad16(input_data)
        self.assertEqual(pad, bytearray(b''))

    def test_example2_1_1(self):
        # Test aus 2.1.1
        s = [0x11111111, 0, 0, 0,
             0x01020304, 0, 0, 0,
             0x9b8d6f43, 0, 0, 0,
             0x01234567, 0, 0, 0]
        chacha20_quarter_round(s, 0, 4, 8, 12)
        self.assertEqual(s[0], 0xea2a92f4)
        self.assertEqual(s[4], 0xcb1cf8ce)
        self.assertEqual(s[8], 0x4581472e)
        self.assertEqual(s[12], 0x5881c4bb)

    def test_example2_2_1(self):
        # Test aus 2.2.1
        s = [0x879531e0, 0xc5ecf37d, 0x516461b1, 0xc9a62f8a,
             0x44c20ef3, 0x3390af7f, 0xd9fc690b, 0x2a5f714c,
             0x53372767, 0xb00a5631, 0x974c541a, 0x359e9963,
             0x5c971061, 0x3d631689, 0x2098d9d6, 0x91dbd320]
        chacha20_quarter_round(s, 2, 7, 8, 13)

        self.assertEqual(s[2], 0xbdb886dc)
        self.assertEqual(s[7], 0xcfacafd2)
        self.assertEqual(s[8], 0xe46bea80)
        self.assertEqual(s[13], 0xccc07c79)

    def test_example2_3_2(self):
        # Test aus 2.3.2
        k = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f.to_bytes(length=32, byteorder='big')
        n = 0x000000090000004a00000000.to_bytes(length=12, byteorder='big')
        c = 1
        init = chacha20_create_initial_state(k, n, c)
        self.assertEqual(init, [0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
                                0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
                                0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c,
                                0x00000001, 0x09000000, 0x4a000000, 0x00000000])
        r = chacha20_block(k, n, c)
        p = int(''.join('''
            10f1e7e4 d13b5915 500fdd1f a32071c4 c7d1f4c7
            33c06803 0422aa9a c3d46c4e d2826446 079faa09
            14c2d705 d98b02a2 b5129cd1 de164eb9 cbd083e8
            a2503c4e
            '''.split()), 16)
        self.assertEqual(r, p)

    def test_example2_4_2(self):
        # Test aus 2.4.2
        k = 0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f.to_bytes(length=32, byteorder='big')
        n = 0x000000000000004a00000000.to_bytes(length=12, byteorder='big')
        c = 1
        plain_text = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, " \
                     "sunscreen would be it."
        r = chacha20_encrypt(k, c, n, plain_text.encode())
        r_ = [0x6e, 0x2e, 0x35, 0x9a, 0x25, 0x68, 0xf9, 0x80, 0x41, 0xba, 0x07, 0x28, 0xdd, 0x0d, 0x69, 0x81,
              0xe9, 0x7e, 0x7a, 0xec, 0x1d, 0x43, 0x60, 0xc2, 0x0a, 0x27, 0xaf, 0xcc, 0xfd, 0x9f, 0xae, 0x0b,
              0xf9, 0x1b, 0x65, 0xc5, 0x52, 0x47, 0x33, 0xab, 0x8f, 0x59, 0x3d, 0xab, 0xcd, 0x62, 0xb3, 0x57,
              0x16, 0x39, 0xd6, 0x24, 0xe6, 0x51, 0x52, 0xab, 0x8f, 0x53, 0x0c, 0x35, 0x9f, 0x08, 0x61, 0xd8,
              0x07, 0xca, 0x0d, 0xbf, 0x50, 0x0d, 0x6a, 0x61, 0x56, 0xa3, 0x8e, 0x08, 0x8a, 0x22, 0xb6, 0x5e,
              0x52, 0xbc, 0x51, 0x4d, 0x16, 0xcc, 0xf8, 0x06, 0x81, 0x8c, 0xe9, 0x1a, 0xb7, 0x79, 0x37, 0x36,
              0x5a, 0xf9, 0x0b, 0xbf, 0x74, 0xa3, 0x5b, 0xe6, 0xb4, 0x0b, 0x8e, 0xed, 0xf2, 0x78, 0x5e, 0x42,
              0x87, 0x4d]
        r_ = bytearray(r_)
        self.assertEqual(r, r_)

    def test_example2_5_2(self):
        # Test aus 2.5.2
        key = 0x85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b.to_bytes(length=32, byteorder='big')
        text = 'Cryptographic Forum Research Group'

        s = calc_s(key)
        self.assertEqual(s, 0x1bf54941aff6bf4afdb20dfb8a800301)

        r = calc_r(key)
        self.assertEqual(r, 0x85d6be7857556d337f4452fe42d506a8)

        r = clamp(r)
        self.assertEqual(r, 0x806d5400e52447c036d555408bed685, 'clamping')

        r = poly1305_mac(text.encode(), key)
        r_ = [0xa8, 0x06, 0x1d, 0xc1, 0x30, 0x51, 0x36, 0xc6, 0xc2, 0x2b, 0x8b, 0xaf, 0x0c, 0x01, 0x27, 0xa9]
        r_ = bytearray(r_)
        self.assertEqual(r, r_)

    def test_example2_6_2(self):
        # Test aus 2.6.2
        key = 0x808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f.to_bytes(length=32, byteorder='big')
        nonce = 0x000000000001020304050607.to_bytes(length=12, byteorder='big')
        r_ = [0x8a, 0xd5, 0xa0, 0x8b, 0x90, 0x5f, 0x81, 0xcc, 0x81, 0x50, 0x40, 0x27, 0x4a, 0xb2, 0x94, 0x71, 0xa8,
              0x33, 0xb6, 0x37, 0xe3, 0xfd, 0x0d, 0xa5, 0x08, 0xdb, 0xb8, 0xe2, 0xfd, 0xd1, 0xa6, 0x46]
        r_ = bytes(r_)

        r = poly1305_key_gen(key, nonce)
        self.assertEqual(r, r_)

    def test_example2_8_2(self):
        # Test aus 2.8.2
        plain_text = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, " \
                     "sunscreen would be it.".encode()
        aad = 0x50515253c0c1c2c3c4c5c6c7.to_bytes(length=12, byteorder='big')
        key = 0x808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f.to_bytes(length=32, byteorder='big')
        iv = 0x4041424344454647.to_bytes(length=8, byteorder='big')
        fixed = 0x07000000.to_bytes(length=4, byteorder='big')
        r_ = (
            bytes(
                [0xd3, 0x1a, 0x8d, 0x34, 0x64, 0x8e, 0x60, 0xdb, 0x7b, 0x86, 0xaf, 0xbc, 0x53, 0xef, 0x7e, 0xc2, 0xa4,
                 0xad, 0xed, 0x51, 0x29, 0x6e, 0x08, 0xfe, 0xa9, 0xe2, 0xb5, 0xa7, 0x36, 0xee, 0x62, 0xd6, 0x3d, 0xbe,
                 0xa4, 0x5e, 0x8c, 0xa9, 0x67, 0x12, 0x82, 0xfa, 0xfb, 0x69, 0xda, 0x92, 0x72, 0x8b, 0x1a, 0x71, 0xde,
                 0x0a, 0x9e, 0x06, 0x0b, 0x29, 0x05, 0xd6, 0xa5, 0xb6, 0x7e, 0xcd, 0x3b, 0x36, 0x92, 0xdd, 0xbd, 0x7f,
                 0x2d, 0x77, 0x8b, 0x8c, 0x98, 0x03, 0xae, 0xe3, 0x28, 0x09, 0x1b, 0x58, 0xfa, 0xb3, 0x24, 0xe4, 0xfa,
                 0xd6, 0x75, 0x94, 0x55, 0x85, 0x80, 0x8b, 0x48, 0x31, 0xd7, 0xbc, 0x3f, 0xf4, 0xde, 0xf0, 0x8e, 0x4b,
                 0x7a, 0x9d, 0xe5, 0x76, 0xd2, 0x65, 0x86, 0xce, 0xc6, 0x4b, 0x61, 0x16]),
            bytes([0x1a, 0xe1, 0x0b, 0x59, 0x4f, 0x09, 0xe2, 0x6a, 0x7e, 0x90, 0x2e, 0xcb, 0xd0, 0x60, 0x06, 0x91])
        )

        r = chacha20_aead_encrypt(aad, key, iv, fixed, plain_text)
        self.assertEqual(r[0], r_[0], 'ciphertext')
        self.assertEqual(r[1], r_[1], 'tag')

        self.assertTrue(chacha20_aead_verify_tag(aad, key, iv, fixed, r[0] + r[1]))
        self.assertFalse(chacha20_aead_verify_tag(aad, key, iv, fixed, r[0] + r[1] + bytes([0, 1, 2, 3])))

        plain_text_ = chacha20_aead_decrypt(aad, key, iv, fixed, r[0] + r[1])
        self.assertEqual(plain_text, plain_text_)

        self.assertFalse(chacha20_aead_decrypt(aad, key, iv, fixed, r[0] + r[1] + bytes([0, 1, 2, 3])))
