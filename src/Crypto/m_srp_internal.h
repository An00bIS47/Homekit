#ifndef  _SRP_INTERNAL_H_
#define _SRP_INTERNAL_H_

/******************************************************
 **  INTERNAL USE ONLY! DON'T RELAY ON THIS STRUCTS  **
 ******************************************************/

/*
 * Secure Remote Password 6a implementation based on mbedtls.
 *
 * Copyright (c) 2019 Stoian Ivanov
 * https://github.com/sdrsdr/mbedtls-csrp
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */



#include "mbedtls/bignum.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

struct NGConstant {
    mbedtls_mpi     *N;
    mbedtls_mpi     *g;
} ;


typedef struct NGHex {
    const char * n_hex;
    const char * g_hex;
} NGHex;

struct SRPKeyPair {
    mbedtls_mpi     *B;
    mbedtls_mpi     *b;
};

typedef union
{
    mbedtls_sha1_context   sha;
    mbedtls_sha256_context sha256;
    mbedtls_sha512_context sha512;
} HashCTX;

struct SRPSession
{
    SRP_HashAlgorithm  hash_alg;
    NGConstant   *ng;
};

struct SRPVerifier
{
    SRP_HashAlgorithm  hash_alg;
    NGConstant  *ng;

    const char          * username;
    int                   authenticated;

    unsigned char M           [SHA512_DIGEST_LENGTH];
    unsigned char H_AMK       [SHA512_DIGEST_LENGTH];
    unsigned char session_key [SHA512_DIGEST_LENGTH];
};


struct SRPUser
{
    SRP_HashAlgorithm  hash_alg;
    NGConstant  *ng;

    mbedtls_mpi *a;
    mbedtls_mpi *A;
    mbedtls_mpi *S;

    int                   authenticated;

    const char *          username;
    const unsigned char * password;
    int                   password_len;

    unsigned char M           [SHA512_DIGEST_LENGTH];
    unsigned char H_AMK       [SHA512_DIGEST_LENGTH];
    unsigned char session_key [SHA512_DIGEST_LENGTH];
};
#endif