/*
 * Secure Remote Password 6a implementation based on mbedtls.
 *
 * Copyright (c) 2019 Stoian Ivanov
 * https://github.com/sdrsdr/mbedtls-csrp
 * 
 * Copyright (c) 2017 Johannes Schriewer
 * https://github.com/dunkelstern/mbedtls-csrp
 *
 * Copyright (c) 2015 Dieter Wimberger
 * https://github.com/dwimberger/mbedtls-csrp
 *
 * Derived from:
 * Copyright (c) 2010 Tom Cocagne. All rights reserved.
 * https://github.com/cocagne/csrp
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Tom Cocagne, Dieter Wimberger
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

#include <time.h>

#include <stdlib.h>
#include <string.h>

#include "mbedtls/bignum.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

#include <esp_log.h>
#include "m_srp.h"
#include "m_srp_internal.h"

#ifdef SRP_TEST
#include "srp_test_config.h"
#include "tutils.h"
#endif

static int g_initialized = 0;
static mbedtls_entropy_context entropy_ctx;
static mbedtls_ctr_drbg_context ctr_drbg_ctx;
static mbedtls_mpi * RR;

#define SRP_BITS_IN_PRIVKEY 256
#define SRP_BYTES_IN_PRIVKEY (SRP_BITS_IN_PRIVKEY/8)
#define SRP_DEFAULT_SALT_BYTES 32


static mbedtls_mpi * H_nn( SRP_HashAlgorithm alg, const mbedtls_mpi * n1, const mbedtls_mpi * n2,int do_pad );




/* All constants here were pulled from Appendix A of RFC 5054 */
static NGHex global_Ng_constants[] = {
 { /* 512 */
  "D66AAFE8E245F9AC245A199F62CE61AB8FA90A4D80C71CD2ADFD0B9DA163B29F2A34AFBDB3B"
  "1B5D0102559CE63D8B6E86B0AA59C14E79D4AA62D1748E4249DF3",
  "2"
 },
 { /* 768 */
   "B344C7C4F8C495031BB4E04FF8F84EE95008163940B9558276744D91F7CC9F402653BE7147F"
   "00F576B93754BCDDF71B636F2099E6FFF90E79575F3D0DE694AFF737D9BE9713CEF8D837ADA"
   "6380B1093E94B6A529A8C6C2BE33E0867C60C3262B",
   "2"
 },
 { /* 1024 */
   "EEAF0AB9ADB38DD69C33F80AFA8FC5E86072618775FF3C0B9EA2314C9C256576D674DF7496"
   "EA81D3383B4813D692C6E0E0D5D8E250B98BE48E495C1D6089DAD15DC7D7B46154D6B6CE8E"
   "F4AD69B15D4982559B297BCF1885C529F566660E57EC68EDBC3C05726CC02FD4CBF4976EAA"
   "9AFD5138FE8376435B9FC61D2FC0EB06E3",
   "2"
 },
 { /* 2048 */
   "AC6BDB41324A9A9BF166DE5E1389582FAF72B6651987EE07FC3192943DB56050A37329CBB4"
   "A099ED8193E0757767A13DD52312AB4B03310DCD7F48A9DA04FD50E8083969EDB767B0CF60"
   "95179A163AB3661A05FBD5FAAAE82918A9962F0B93B855F97993EC975EEAA80D740ADBF4FF"
   "747359D041D5C33EA71D281E446B14773BCA97B43A23FB801676BD207A436C6481F1D2B907"
   "8717461A5B9D32E688F87748544523B524B0D57D5EA77A2775D2ECFA032CFBDBF52FB37861"
   "60279004E57AE6AF874E7303CE53299CCC041C7BC308D82A5698F3A8D0C38271AE35F8E9DB"
   "FBB694B5C803D89F7AE435DE236D525F54759B65E372FCD68EF20FA7111F9E4AFF73",
   "2"
 },
 { /* 3072 */
   "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08"
   "8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B"
   "302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9"
   "A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6"
   "49286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8"
   "FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D"
   "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C"
   "180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
   "3995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
   "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7D"
   "B3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D226"
   "1AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200C"
   "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFC"
   "E0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF",
    "5"
 },
 { /* 4096 */
   "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08"
   "8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B"
   "302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9"
   "A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6"
   "49286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8"
   "FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D"
   "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C"
   "180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
   "3995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
   "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7D"
   "B3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D226"
   "1AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200C"
   "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFC"
   "E0FD108E4B82D120A92108011A723C12A787E6D788719A10BDBA5B26"
   "99C327186AF4E23C1A946834B6150BDA2583E9CA2AD44CE8DBBBC2DB"
   "04DE8EF92E8EFC141FBECAA6287C59474E6BC05D99B2964FA090C3A2"
   "233BA186515BE7ED1F612970CEE2D7AFB81BDD762170481CD0069127"
   "D5B05AA993B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934063199"
   "FFFFFFFFFFFFFFFF",
   "5"
 },
 { /* 8192 */
   "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E08"
   "8A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B"
   "302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9"
   "A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE6"
   "49286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8"
   "FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D"
   "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C"
   "180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
   "3995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
   "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7D"
   "B3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D226"
   "1AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200C"
   "BBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFC"
   "E0FD108E4B82D120A92108011A723C12A787E6D788719A10BDBA5B26"
   "99C327186AF4E23C1A946834B6150BDA2583E9CA2AD44CE8DBBBC2DB"
   "04DE8EF92E8EFC141FBECAA6287C59474E6BC05D99B2964FA090C3A2"
   "233BA186515BE7ED1F612970CEE2D7AFB81BDD762170481CD0069127"
   "D5B05AA993B4EA988D8FDDC186FFB7DC90A6C08F4DF435C934028492"
   "36C3FAB4D27C7026C1D4DCB2602646DEC9751E763DBA37BDF8FF9406"
   "AD9E530EE5DB382F413001AEB06A53ED9027D831179727B0865A8918"
   "DA3EDBEBCF9B14ED44CE6CBACED4BB1BDB7F1447E6CC254B33205151"
   "2BD7AF426FB8F401378CD2BF5983CA01C64B92ECF032EA15D1721D03"
   "F482D7CE6E74FEF6D55E702F46980C82B5A84031900B1C9E59E7C97F"
   "BEC7E8F323A97A7E36CC88BE0F1D45B7FF585AC54BD407B22B4154AA"
   "CC8F6D7EBF48E1D814CC5ED20F8037E0A79715EEF29BE32806A1D58B"
   "B7C5DA76F550AA3D8A1FBFF0EB19CCB1A313D55CDA56C9EC2EF29632"
   "387FE8D76E3C0468043E8F663F4860EE12BF2D5B0B7474D6E694F91E"
   "6DBE115974A3926F12FEE5E438777CB6A932DF8CD8BEC4D073B931BA"
   "3BC832B68D9DD300741FA7BF8AFC47ED2576F6936BA424663AAB639C"
   "5AE4F5683423B4742BF1C978238F16CBE39D652DE3FDB8BEFC848AD9"
   "22222E04A4037C0713EB57A81A23F0C73473FC646CEA306B4BCBC886"
   "2F8385DDFA9D4B7FA2C087E879683303ED5BDD3A062B3CF5B3A278A6"
   "6D2A13F83F44F82DDF310EE074AB6A364597E899A0255DC164F31CC5"
   "0846851DF9AB48195DED7EA1B1D510BD7EE74D73FAF36BC31ECFA268"
   "359046F4EB879F924009438B481C6CD7889A002ED5EE382BC9190DA6"
   "FC026E479558E4475677E9AA9E3050E2765694DFC81F56E880B96E71"
   "60C980DD98EDD3DFFFFFFFFFFFFFFFFF",
   "13"
 },
 {0,0} /* null sentinel */
};


NGConstant * srp_ng_new( SRP_NGType ng_type, const char * n_hex, const char * g_hex )
{
	if ((unsigned)ng_type>=(unsigned)SRP_NG_LAST) return NULL;

    NGConstant * ng   = (NGConstant *) malloc( sizeof(NGConstant) );
    if( !ng )
       return NULL;

    ng->N = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!ng->N) { 
		free(ng); 
		return NULL;
	}
    ng->g = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if( !ng->g ) {
		free(ng->N);
		free(ng);
		return 0;
	}

    mbedtls_mpi_init(ng->N);
    mbedtls_mpi_init(ng->g);


    if ( ng_type != SRP_NG_CUSTOM )
    {
        n_hex = global_Ng_constants[ ng_type ].n_hex;
        g_hex = global_Ng_constants[ ng_type ].g_hex;
    }

    mbedtls_mpi_read_string( ng->N, 16, n_hex);
    mbedtls_mpi_read_string( ng->g, 16, g_hex);

    return ng;
}

NGConstant * srp_ng_new1( NGConstant * copy_from_ng)
{
    NGConstant * ng   = (NGConstant *) malloc( sizeof(NGConstant) );
    if( !ng ) {
		return 0;
	}

    ng->N = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    ng->g = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));

    if( !ng->N || !ng->g ) {
		if (ng->N) free(ng->N);
		if (ng->g) free(ng->g);
		free(ng);
		return 0;
	}
    mbedtls_mpi_init(ng->N);
    mbedtls_mpi_init(ng->g);

	if(!(mbedtls_mpi_copy(ng->N,copy_from_ng->N)==0  && mbedtls_mpi_copy(ng->g,copy_from_ng->g)==0)){
		srp_ng_delete(ng);
		return 0;
	}

    return ng;
}

void srp_ng_delete( NGConstant * ng )
{
   if (ng)
   {
      mbedtls_mpi_free( ng->N );
      mbedtls_mpi_free( ng->g );
      free(ng->N);
      free(ng->g);
      free(ng);
   }
}



SRPKeyPair * srp_keypair_new(SRPSession *session,const unsigned char * bytes_v, int len_v, const unsigned char ** bytes_B, int * len_B){

    mbedtls_mpi *k= 0;
    mbedtls_mpi *tmp1=0;
    mbedtls_mpi *tmp2=0;
    mbedtls_mpi *v=0;
	SRPKeyPair * keys=0;


    k = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!k) goto cleanup;
    mbedtls_mpi_init(k);

    tmp1 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!tmp1) goto cleanup;
    mbedtls_mpi_init(tmp1);

    tmp2 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!tmp1) goto cleanup;
    mbedtls_mpi_init(tmp2);

    v = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!tmp1) goto cleanup;
    mbedtls_mpi_init(v);
	if(mbedtls_mpi_read_binary( v, bytes_v, len_v )!=0) goto cleanup;


	keys   = (SRPKeyPair *) malloc( sizeof(SRPKeyPair) );
	if (!keys) goto cleanup;

    keys->B = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    keys->b = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (keys->B ==0 || keys->b==0){
		if (keys->B) free (keys->B);
		if (keys->b) free (keys->b);
		free(keys);
		keys=0;
		goto cleanup;
	}
    mbedtls_mpi_init(keys->B);
    mbedtls_mpi_init(keys->b);

#ifdef SRP_TEST_FIXED_b
	mbedtls_mpi_read_string(keys->b,16,SRP_TEST_FIXED_b_STR);
#else 
	mbedtls_mpi_fill_random( keys->b, SRP_BYTES_IN_PRIVKEY,
                     &mbedtls_ctr_drbg_random,
                     &ctr_drbg_ctx );
#endif
	k = H_nn(session->hash_alg, session->ng->N, session->ng->g,1);

	/* B = kv + g^b */
	mbedtls_mpi_mul_mpi( tmp1, k, v);
	mbedtls_mpi_exp_mod( tmp2, session->ng->g, keys->b, session->ng->N, RR );
	mbedtls_mpi_add_mpi( tmp1, tmp1, tmp2 );
	mbedtls_mpi_mod_mpi( keys->B, tmp1, session->ng->N );

#ifdef SRP_TEST_PRINT_b
	tutils_mpi_print ("server priv (b)",keys->b);
#endif
#ifdef SRP_TEST_PRINT_B
	tutils_mpi_print ("server pub (B)",keys->B);
#endif

	if (bytes_B) {
		*len_B   = mbedtls_mpi_size(keys->B);
		*bytes_B = malloc( *len_B );

		if( !*bytes_B ){
			if (keys->B) free (keys->B);
			if (keys->b) free (keys->b);
			free(keys);
			keys=0;
			goto cleanup;
		}
		mbedtls_mpi_write_binary( keys->B, (unsigned char *)*bytes_B, *len_B );
	}

cleanup:
	if (tmp1) {
		mbedtls_mpi_free(tmp1);
		free(tmp1);
	}

	if (tmp2) {
		mbedtls_mpi_free(tmp2);
		free(tmp2);
	}

	if (v) {
		mbedtls_mpi_free(v);
		free(v);
	}

	if (k) {
		mbedtls_mpi_free(k);
		free(k);
	}
	return keys;
}

void srp_keypair_delete( SRPKeyPair * keys ) {
	if (keys) {
		mbedtls_mpi_free( keys->B );
		mbedtls_mpi_free( keys->b );
		free(keys->B);
		free(keys->b);
		free(keys);
	}
}



static void hash_init( SRP_HashAlgorithm alg, HashCTX *c )
{
	switch (alg) {
		case SRP_SHA1  : {
			mbedtls_sha1_init( &c->sha );
			mbedtls_sha1_starts( &c->sha );
			break;
		}
		case SRP_SHA224:
		case SRP_SHA256: {
			mbedtls_sha256_init( &c->sha256 );
			if (alg==SRP_SHA224) {
				mbedtls_sha256_starts( &c->sha256, 1 );
			} else {
				mbedtls_sha256_starts( &c->sha256, 0 );
			}
			break;
		}
		case SRP_SHA384:
		case SRP_SHA512:{
			mbedtls_sha512_init( &c->sha512 );
			if (alg==SRP_SHA384) {
				mbedtls_sha512_starts( &c->sha512, 1 );
			} else {
				mbedtls_sha512_starts( &c->sha512, 0 );
			}
			break;
		}
    	default:
			return;
	};
}


static void hash_update( SRP_HashAlgorithm alg, HashCTX *c, const void *data, size_t len )
{
    switch (alg)
    {
      case SRP_SHA1  : mbedtls_sha1_update( &c->sha, data, len ); break;
      case SRP_SHA224: mbedtls_sha256_update( &c->sha256, data, len ); break;
      case SRP_SHA256: mbedtls_sha256_update( &c->sha256, data, len ); break;
      case SRP_SHA384: mbedtls_sha512_update( &c->sha512, data, len ); break;
      case SRP_SHA512: mbedtls_sha512_update( &c->sha512, data, len ); break;
      default:
        return;
    };
}
static void hash_final( SRP_HashAlgorithm alg, HashCTX *c, unsigned char *md )
{
    switch (alg)
    {
      case SRP_SHA1  : mbedtls_sha1_finish( &c->sha, md ); break;
      case SRP_SHA224: mbedtls_sha256_finish( &c->sha256, md ); break;
      case SRP_SHA256: mbedtls_sha256_finish( &c->sha256, md ); break;
      case SRP_SHA384: mbedtls_sha512_finish( &c->sha512, md ); break;
      case SRP_SHA512: mbedtls_sha512_finish( &c->sha512, md ); break;
      default:
        return;
    };
}
static void hash( SRP_HashAlgorithm alg, const unsigned char *d, size_t n, unsigned char *md )
{
    switch (alg)
    {
      case SRP_SHA1  : mbedtls_sha1( d, n, md ); break;
      case SRP_SHA224: mbedtls_sha256( d, n, md, 1); break;
      case SRP_SHA256: mbedtls_sha256( d, n, md, 0); break;
      case SRP_SHA384: mbedtls_sha512( d, n, md, 1 ); break;
      case SRP_SHA512: mbedtls_sha512( d, n, md, 0 ); break;
      default:
        return;
    };
}
static int hash_length( SRP_HashAlgorithm alg )
{
    switch (alg)
    {
      case SRP_SHA1  : return SHA1_DIGEST_LENGTH;
      case SRP_SHA224: return SHA224_DIGEST_LENGTH;
      case SRP_SHA256: return SHA256_DIGEST_LENGTH;
      case SRP_SHA384: return SHA384_DIGEST_LENGTH;
      case SRP_SHA512: return SHA512_DIGEST_LENGTH;
      default:
        return -1;
    };
}
int srp_hash_length( SRPSession *ses ) {
	return hash_length(ses->hash_alg);
}

static mbedtls_mpi * H_nn( SRP_HashAlgorithm alg, const mbedtls_mpi * n1, const mbedtls_mpi * n2,int do_pad )
{
    unsigned char   buff[ SHA512_DIGEST_LENGTH ];
    int             len_n1 = mbedtls_mpi_size(n1);
    int             len_n2 = mbedtls_mpi_size(n2);
	int             nbytes = len_n1 + len_n2;
	int kldiff;
	if (do_pad) {
		kldiff= len_n1 - len_n2;
		if (kldiff<0) return 0;
		nbytes +=kldiff;
	} else kldiff=0;
    unsigned char * bin    = (unsigned char *) malloc( nbytes );
    if (!bin)
       return 0;
	if (kldiff) memset(bin+len_n1,0,kldiff);
    mbedtls_mpi_write_binary( n1, bin, len_n1 );
    mbedtls_mpi_write_binary( n2, bin+len_n1+kldiff, len_n2 );
    hash( alg, bin, nbytes, buff );
    free(bin);
    mbedtls_mpi * bn;
    bn = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(bn);
    mbedtls_mpi_read_binary( bn, buff, hash_length(alg) );
    return bn;
}

static mbedtls_mpi * H_ns( SRP_HashAlgorithm alg, const mbedtls_mpi * n, const unsigned char * bytes, int len_bytes )
{
    unsigned char   buff[ SHA512_DIGEST_LENGTH ];
    int             len_n  = mbedtls_mpi_size(n);
    int             nbytes = len_n + len_bytes;
    unsigned char * bin    = (unsigned char *) malloc( nbytes );
    if (!bin)
       return 0;
    mbedtls_mpi_write_binary( n, bin, len_n );
    memcpy( bin + len_n, bytes, len_bytes );
    hash( alg, bin, nbytes, buff );
    free(bin);

    mbedtls_mpi * bn;
    bn = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(bn);
    mbedtls_mpi_read_binary( bn, buff, hash_length(alg) );
    return bn;
}

static mbedtls_mpi * calculate_x( SRP_HashAlgorithm alg, const mbedtls_mpi * salt, const char * username, const unsigned char * password, int password_len )
{
    unsigned char ucp_hash[SHA512_DIGEST_LENGTH];
    HashCTX       ctx;

#ifdef SRP_TEST_SHA512
    hash_init( alg, &ctx );
    hash_update( alg, &ctx, "abc",3);
    hash_final( alg, &ctx, ucp_hash );
	tutils_array_print("VAR:test sha512",ucp_hash, hash_length(alg));
#endif

    hash_init( alg, &ctx );
    hash_update( alg, &ctx, username, strlen(username) );
    hash_update( alg, &ctx, ":", 1 );
    hash_update( alg, &ctx, password, password_len );

    hash_final( alg, &ctx, ucp_hash );
#ifdef SRP_TEST_DBG_VER
	tutils_array_print("VER:username",username, strlen(username));
	tutils_array_print("VAR:password",password, password_len);
	tutils_mpi_print("VAR:salt",salt);
	tutils_array_print("VAR:ucp_hash",ucp_hash, hash_length(alg));
#endif
    return H_ns( alg, salt, ucp_hash, hash_length(alg) );
}

static void update_hash_n( SRP_HashAlgorithm alg, HashCTX *ctx, const mbedtls_mpi * n )
{
    unsigned long len = mbedtls_mpi_size(n);
    unsigned char * n_bytes = (unsigned char *) malloc( len );
    if (!n_bytes)
       return;
    mbedtls_mpi_write_binary( n, n_bytes, len );
    hash_update(alg, ctx, n_bytes, len);
    free(n_bytes);
}

static void hash_num( SRP_HashAlgorithm alg, const mbedtls_mpi * n, unsigned char * dest )
{
    int             nbytes = mbedtls_mpi_size(n);
    unsigned char * bin    = (unsigned char *) malloc( nbytes );
    if(!bin)
       return;
    mbedtls_mpi_write_binary( n, bin, nbytes );
    hash( alg, bin, nbytes, dest );
    free(bin);
}

static void calculate_M( SRP_HashAlgorithm alg, NGConstant *ng, unsigned char * dest, const char * I, const mbedtls_mpi * s,
                         const mbedtls_mpi * A, const mbedtls_mpi * B, const unsigned char * K )
{
    unsigned char H_N[ SHA512_DIGEST_LENGTH ];
    unsigned char H_g[ SHA512_DIGEST_LENGTH ];
    unsigned char H_I[ SHA512_DIGEST_LENGTH ];
    unsigned char H_xor[ SHA512_DIGEST_LENGTH ];
    HashCTX       ctx;
    int           i = 0;
    int           hash_len = hash_length(alg);

    hash_num( alg, ng->N, H_N );
    hash_num( alg, ng->g, H_g );

    hash(alg, (const unsigned char *)I, strlen(I), H_I);


    for (i=0; i < hash_len; i++ )
        H_xor[i] = H_N[i] ^ H_g[i];

    hash_init( alg, &ctx );

    hash_update( alg, &ctx, H_xor, hash_len );
    hash_update( alg, &ctx, H_I,   hash_len );
    update_hash_n( alg, &ctx, s );
    update_hash_n( alg, &ctx, A );
    update_hash_n( alg, &ctx, B );
    hash_update( alg, &ctx, K, hash_len );

    hash_final( alg, &ctx, dest );
}

static void calculate_H_AMK( SRP_HashAlgorithm alg, unsigned char *dest, const mbedtls_mpi * A, const unsigned char * M, const unsigned char * K )
{
    HashCTX ctx;

    hash_init( alg, &ctx );

    update_hash_n( alg, &ctx, A );
    hash_update( alg, &ctx, M, hash_length(alg) );
    hash_update( alg, &ctx, K, hash_length(alg) );

    hash_final( alg, &ctx, dest );
}


static void init_random()
{
    if (g_initialized)
        return;

     mbedtls_entropy_init( &entropy_ctx );
     mbedtls_ctr_drbg_init( &ctr_drbg_ctx );

     unsigned char hotBits[128] = {
    82, 42, 71, 87, 124, 241, 30, 1, 54, 239, 240, 121, 89, 9, 151, 11, 60,
    226, 142, 47, 115, 157, 100, 126, 242, 132, 46, 12, 56, 197, 194, 76,
    198, 122, 90, 241, 255, 43, 120, 209, 69, 21, 195, 212, 100, 251, 18,
    111, 30, 238, 24, 199, 238, 236, 138, 225, 45, 15, 42, 83, 114, 132,
    165, 141, 32, 185, 167, 100, 131, 23, 236, 9, 11, 51, 130, 136, 97, 161,
    36, 174, 129, 234, 2, 54, 119, 184, 70, 103, 118, 109, 122, 15, 24, 23,
    166, 203, 102, 160, 77, 100, 17, 4, 132, 138, 215, 204, 109, 245, 122,
    9, 184, 89, 70, 247, 125, 97, 213, 240, 85, 243, 91, 226, 127, 64, 136,
    37, 154, 232
};

     mbedtls_ctr_drbg_seed(
        &ctr_drbg_ctx,
        mbedtls_entropy_func,
        &entropy_ctx,
        hotBits,
        128
    );

    RR = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(RR);
    g_initialized = 1;

}


/***********************************************************************************************************
 *
 *  Exported Functions
 *
 ***********************************************************************************************************/

SRPSession * srp_session_new( SRP_HashAlgorithm alg,
                                     SRP_NGType ng_type,
                                     const char * n_hex, const char * g_hex)
{
	if ((unsigned)alg>=(unsigned)SRP_SHA_LAST) return NULL;
	if ((unsigned)ng_type>=(unsigned)SRP_NG_LAST) return NULL;

    SRPSession * session;

    session = (SRPSession *)malloc(sizeof(SRPSession));

	if (!session) return NULL;

    memset(session, 0, sizeof(SRPSession));

    session->hash_alg = alg;
    session->ng  = srp_ng_new( ng_type, n_hex, g_hex );
	if (!session->ng) {
		free(session);
		return NULL;
	}
    init_random(); /* Only happens once */

    return session;
}

void srp_session_delete(SRPSession *session)
{
    srp_ng_delete( session->ng );
    free(session);
}

int srp_random_seeded(){
	return g_initialized;
}

void srp_random_seed( const unsigned char * random_data, int data_length )
{
    g_initialized = 1;


   if( mbedtls_ctr_drbg_seed( &ctr_drbg_ctx, mbedtls_entropy_func, &entropy_ctx,
                           (const unsigned char *) random_data,
                           data_length )  != 0 )
    {
        return;
    }

}

void srp_create_salted_verification_key( SRPSession *session,
                                         const char * username,
                                         const unsigned char * password, int len_password,
                                         const unsigned char ** bytes_s, int * len_s,
                                         const unsigned char ** bytes_v, int * len_v)
{
	*len_s=SRP_DEFAULT_SALT_BYTES;
	srp_create_salted_verification_key1(session,username,password,len_password,bytes_s,SRP_DEFAULT_SALT_BYTES,bytes_v,len_v);
}

void srp_create_salted_verification_key1( SRPSession *session,
                                         const char * username,
                                         const unsigned char * password, int len_password,
                                         const unsigned char ** bytes_s, int  len_s,
                                         const unsigned char ** bytes_v, int * len_v)
{

	*bytes_s=NULL;
	*bytes_v=NULL;
	if( !session) return;

    mbedtls_mpi     * s=NULL;
    mbedtls_mpi     * v=NULL;
    mbedtls_mpi     * x=NULL;

    s = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (s) {
	    mbedtls_mpi_init(s);
	} else {
		goto cleanup_and_exit;
	}

    v = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if( v ){
	    mbedtls_mpi_init(v);
	} else {
		goto cleanup_and_exit;
	}
#ifdef SRP_TEST_FIXED_SALT
	mbedtls_mpi_read_string(s,16,SRP_TEST_FIXED_SALT_STR);
#else
    mbedtls_mpi_fill_random( s, len_s,
                     &mbedtls_ctr_drbg_random,
                     &ctr_drbg_ctx );
#endif

#ifdef SRP_TEST_PRINT_SALT
	tutils_mpi_print ("salt (s)",s);
#endif

    x = calculate_x( session->hash_alg, s, username, password, len_password );

    if( !x )
       goto cleanup_and_exit;

    mbedtls_mpi_exp_mod(v, session->ng->g, x, session->ng->N, RR);

#ifdef SRP_TEST_PRINT_v
	tutils_mpi_print ("verifier (v)",v);
#endif

    *bytes_s = (const unsigned char *) malloc( len_s );
	if (*bytes_s==NULL) {
		 goto cleanup_and_exit;
	}

    *len_v   = mbedtls_mpi_size(v);
    *bytes_v = (const unsigned char *) malloc( *len_v );
	if (*bytes_v==NULL) {
		free((void*)(*bytes_s));
		*bytes_s=NULL;
		 goto cleanup_and_exit;
	}



    mbedtls_mpi_write_binary( s, (unsigned char *)*bytes_s, len_s );
    mbedtls_mpi_write_binary( v, (unsigned char *)*bytes_v, *len_v );

 cleanup_and_exit:
	if (s) {
    	mbedtls_mpi_free(s);
    	free(s);
	}

    if (v) {
		mbedtls_mpi_free(v);
    	free(v);
	}

    if (x) {
		mbedtls_mpi_free(x);
	    free(x);
	}
    //TODO: BN_CTX_free(ctx);
}



/* Out: bytes_B, len_B.
 *
 * On failure, bytes_B will be set to NULL and len_B will be set to 0
 */
SRPVerifier *  srp_verifier_new( SRPSession *session,
                                        const char *username,
                                        const unsigned char * bytes_s, int len_s,
                                        const unsigned char * bytes_v, int len_v,
                                        const unsigned char * bytes_A, int len_A,
                                        const unsigned char ** bytes_B, int * len_B)
{
	return srp_verifier_new1(
		session,username,1,bytes_s,len_s,bytes_v,len_v,
		bytes_A,len_A, bytes_B,len_B,
		NULL
	);
}

/* Out: bytes_B, len_B if not using SRPKeyPair keys
 *
 * On failure, bytes_B will be set to NULL and len_B will be set to 0, keys=NULL is OK!
 * bytes_B=NULL is OK 
 */
SRPVerifier *  srp_verifier_new1( SRPSession *session,
                                        const char *username, int copy_username,
                                        const unsigned char * bytes_s, int len_s,
                                        const unsigned char * bytes_v, int len_v,
                                        const unsigned char * bytes_A, int len_A,
                                        const unsigned char ** bytes_B, int * len_B,
                                        SRPKeyPair *keys)
{
	if (bytes_B) {
		*bytes_B=NULL;
		*len_B=0;
	}

	if( session==NULL ) {

        ESP_LOGE("SRP", "session is null\n");
        return NULL;
    }

    mbedtls_mpi *s;
    s = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(s);
    mbedtls_mpi_read_binary(s, bytes_s, len_s);

    mbedtls_mpi *v;
    v = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(v);
    mbedtls_mpi_read_binary(v, bytes_v, len_v);

    mbedtls_mpi *A;
    A = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(A);
    mbedtls_mpi_read_binary(A, bytes_A, len_A);

    mbedtls_mpi             *u    = 0;


    mbedtls_mpi             *S;
    S = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(S);


    //mbedtls_mpi             *k    = 0;

    mbedtls_mpi             *tmp1;
    tmp1 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(tmp1);

    mbedtls_mpi             *tmp2;
    tmp2 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    mbedtls_mpi_init(tmp2);


    SRPVerifier *ver ;
    ver = (SRPVerifier *) malloc( sizeof(SRPVerifier) );

    if(!S || !tmp1 || !tmp2 || !ver ) {
		if (ver) { 
            free(ver); 
            ver = NULL; 
        }
        ESP_LOGE("SRP", "!S || !tmp1 || !tmp2 || !ver is NULL\n");
        goto cleanup_and_exit;
    }

	memset(ver,0,sizeof(SRPVerifier));
    ver->hash_alg = session->hash_alg;
    ver->ng       = session->ng;

	if (copy_username){
		int ulen = strlen(username) + 1;
		ver->username = (char *) malloc( sizeof(char) * ulen ); // FIXME
		if (!ver->username) {
			free(ver);
			ver = 0;

            ESP_LOGE("SRP", "ver->username is NULL\n");
			goto cleanup_and_exit;
		}
		memcpy( (char*)ver->username, username, ulen );
	}

    /* SRP-6a safety check */
    mbedtls_mpi_mod_mpi( tmp1, A, session->ng->N );
    if ( mbedtls_mpi_cmp_int( tmp1, 0 )  != 0)
    {
		int temp_keys;
		if (keys==NULL) {
			temp_keys=1;
            ESP_LOGE("SRP", "keys is NULL\n");
			keys=srp_keypair_new(session,bytes_v,len_v,bytes_B,len_B);
			if (keys==NULL) {                
                goto cleanup_and_exit;
            }
		} else temp_keys=0;
		/*
        mbedtls_mpi_fill_random( b, SRP_BYTES_IN_PRIVKEY,
                     &mbedtls_ctr_drbg_random,
                     &ctr_drbg_ctx );

       //k = H_nn(session->hash_alg, session->ng->N, session->ng->g);

       // B = kv + g^b 
       //mbedtls_mpi_mul_mpi( tmp1, k, v);
       //mbedtls_mpi_exp_mod( tmp2, session->ng->g, b, session->ng->N, RR );
       //mbedtls_mpi_add_mpi( tmp1, tmp1, tmp2 );
       //mbedtls_mpi_mod_mpi( B, tmp1, session->ng->N );
		*/

       u = H_nn(session->hash_alg, A, keys->B,1); 

       /* S = (A *(v^u)) ^ b */
       mbedtls_mpi_exp_mod(tmp1, v, u, session->ng->N, RR);
       mbedtls_mpi_mul_mpi(tmp2, A, tmp1);
       mbedtls_mpi_exp_mod(S, tmp2, keys->b, session->ng->N, RR);

       hash_num(session->hash_alg, S, ver->session_key);

       calculate_M( session->hash_alg, session->ng, ver->M, username, s, A, keys->B, ver->session_key );
       calculate_H_AMK( session->hash_alg, ver->H_AMK, A, ver->M, ver->session_key );

		if (temp_keys) srp_keypair_delete(keys);
    }

 cleanup_and_exit:
    mbedtls_mpi_free(s);
    free(s);
    mbedtls_mpi_free(v);
    free(v);
    mbedtls_mpi_free(A);
    free(A);
    if (u) {mbedtls_mpi_free(u); free(u);}

    mbedtls_mpi_free(S);
    free(S);
    mbedtls_mpi_free(tmp1);
    free(tmp1);
    mbedtls_mpi_free(tmp2);
    free(tmp2);

    return ver;
}




void srp_verifier_delete( SRPVerifier * ver ){
	if (ver) {
		srp_ng_delete( ver->ng );
		if (ver->username) free( (char *) ver->username );
		memset(ver, 0, sizeof(*ver));
		free( ver );
	}
}



int srp_verifier_is_authenticated( SRPVerifier * ver )
{
    return ver->authenticated;
}


const char * srp_verifier_get_username( SRPVerifier * ver )
{
    return ver->username;
}


const unsigned char * srp_verifier_get_session_key( SRPVerifier * ver, int * key_length )
{
    if (key_length)
        *key_length = hash_length( ver->hash_alg );
    return ver->session_key;
}

int srp_session_get_key_length( SRPSession * ses ){
	return hash_length( ses->hash_alg );
}

int srp_verifier_get_session_key_length( SRPVerifier * ver )
{
    return hash_length( ver->hash_alg );
}


/* user_M,bytes_HAMK are digest generated with session selected hash */
int srp_verifier_verify_session( SRPVerifier * ver, const unsigned char * user_M, const unsigned char ** bytes_HAMK )
{
    if ( memcmp( ver->M, user_M, hash_length(ver->hash_alg) ) == 0 )
    {
        ver->authenticated = 1;
        if (bytes_HAMK) *bytes_HAMK = ver->H_AMK;
		return 1;
    }
    else {
        if (bytes_HAMK) *bytes_HAMK = NULL;
		return 0;
	}
}

/* return bytes_HAMK which is  digest generated with session selected hash */
const unsigned char * srp_verifier_get_HAMK( SRPVerifier * ver) {
	return ver->H_AMK;
}
/*******************************************************************************/

SRPUser * srp_user_new(
	SRPSession *session, const char * username,
	const unsigned char * bytes_password, int len_password
) {
	NGConstant *ng=srp_ng_new1(session->ng);
	if (!ng) return NULL;
	//srp_user_new1 takse ownership of ng
	return srp_user_new1(session->hash_alg,ng, username,bytes_password,len_password);
}

//we take wonership of ng here so please don't free in your code
SRPUser * srp_user_new1(
	SRP_HashAlgorithm  hash_alg, NGConstant *ng,
	const char * username, const unsigned char * bytes_password, int len_password
) {

	if ((unsigned)hash_alg>=(unsigned)SRP_SHA_LAST) return NULL;
	if (ng==NULL) return NULL;

	SRPUser  *usr  = (SRPUser *) malloc( sizeof(SRPUser) );
	int ulen = strlen(username) + 1;

	if (!usr) goto err_exit;
	memset(usr,0,sizeof(SRPUser));


	usr->hash_alg = hash_alg;
	usr->ng       = ng;

	usr->a = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!usr->a) goto err_exit;
	mbedtls_mpi_init(usr->a);

	usr->A = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!usr->A) goto err_exit;
	mbedtls_mpi_init(usr->A);

	usr->S = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
	if (!usr->S) goto err_exit;
	mbedtls_mpi_init(usr->S);

	usr->username     = (const char *) malloc(ulen);
	if (!usr->username) goto err_exit;
	memcpy((char *)usr->username, username, ulen);

	usr->password_len = len_password;
	usr->password = (const unsigned char *) malloc(len_password);
	if (!usr->password) goto err_exit;
	memcpy((char *)usr->password, bytes_password, len_password);

	usr->authenticated = 0;

	return usr;

err_exit:
	if (usr) {
		if (usr->a) {
			mbedtls_mpi_free(usr->a);
			free(usr->a);
		}

		if (usr->A) {
			mbedtls_mpi_free(usr->A);
			free(usr->A);
		}
		if (usr->S) {
			mbedtls_mpi_free(usr->S);
			free(usr->S);
		}
		if (usr->ng) srp_ng_delete(usr->ng);
		if (usr->username) {
			memset((void*)usr->username, 0, ulen);
			free((void*)usr->username);
		}
		if (usr->password) {
			memset((void*)usr->password, 0, usr->password_len);
			free((void*)usr->password);
		}
		free(usr);
	}

	return 0;
}



void srp_user_delete( SRPUser * usr )
{
	if( !usr ) return;
	mbedtls_mpi_free( usr->a ); free( usr->a );
	mbedtls_mpi_free( usr->A ); free( usr->A );
	mbedtls_mpi_free( usr->S ); free( usr->S );
	
	srp_ng_delete( usr->ng );

	memset((void*)usr->password, 0, usr->password_len);

	free((char *)usr->username);
	free((char *)usr->password);


	memset(usr, 0, sizeof(SRPUser));
	free( usr );
}



int srp_user_is_authenticated( SRPUser * usr)
{
    return usr->authenticated;
}


const char * srp_user_get_username( SRPUser * usr )
{
    return usr->username;
}



const unsigned char * srp_user_get_session_key( SRPUser * usr, int * key_length )
{
    if (key_length)
        *key_length = hash_length( usr->hash_alg );
    return usr->session_key;
}


int                   srp_user_get_session_key_length( SRPUser * usr )
{
    return hash_length( usr->hash_alg );
}



/* Output: username, bytes_A, len_A */
void  srp_user_start_authentication( SRPUser * usr, const char ** username,
                                     const unsigned char ** bytes_A, int * len_A )
{

#ifdef SRP_TEST_FIXED_a
	mbedtls_mpi_read_string(usr->a, 16,SRP_TEST_FIXED_a_STR);
#else
	mbedtls_mpi_fill_random( usr->a, SRP_BYTES_IN_PRIVKEY, &mbedtls_ctr_drbg_random, &ctr_drbg_ctx);
#endif
	mbedtls_mpi_exp_mod(usr->A, usr->ng->g, usr->a, usr->ng->N, RR);

#ifdef SRP_TEST_PRINT_a
	tutils_mpi_print ("server priv (a)",usr->a);
#endif
#ifdef SRP_TEST_PRINT_A
	tutils_mpi_print ("server pub (A)",usr->A);
#endif


	*len_A   = mbedtls_mpi_size(usr->A);
	*bytes_A = malloc( *len_A );

	if (!*bytes_A) {
		*len_A = 0;
		if (username) *username = NULL;
		return;
	}

	mbedtls_mpi_write_binary( usr->A, (unsigned char *) *bytes_A, *len_A );

	if (username) *username = usr->username;
}


/* Output: bytes_M. Buffer length is SHA512_DIGEST_LENGTH */
void  srp_user_process_challenge( SRPUser * usr,
                                  const unsigned char * bytes_s, int len_s,
                                  const unsigned char * bytes_B, int len_B,
                                  const unsigned char ** bytes_M, int * len_M )
{
    mbedtls_mpi *u = NULL;
    mbedtls_mpi *x = NULL;
    mbedtls_mpi *k = NULL;

    mbedtls_mpi *s = NULL;
    mbedtls_mpi *B = NULL;
    mbedtls_mpi *v = NULL;
    mbedtls_mpi *tmp1 = NULL;
    mbedtls_mpi *tmp2 = NULL;
    mbedtls_mpi *tmp3 = NULL;
    *len_M = 0;
    *bytes_M = NULL;

    s = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if( !s ) goto cleanup_and_exit;
    mbedtls_mpi_init(s);
    mbedtls_mpi_read_binary(s, bytes_s, len_s);

    B = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if( !B ) goto cleanup_and_exit;
    mbedtls_mpi_init(B);
    mbedtls_mpi_read_binary(B, bytes_B, len_B);

    v = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if(  !v ) goto cleanup_and_exit;
    mbedtls_mpi_init(v);

    tmp1 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if(  !tmp1 ) goto cleanup_and_exit;
    mbedtls_mpi_init(tmp1);

    tmp2 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if(  !tmp2 ) goto cleanup_and_exit;
    mbedtls_mpi_init(tmp2);

    tmp3 = (mbedtls_mpi *) malloc(sizeof(mbedtls_mpi));
    if(  !tmp3 ) goto cleanup_and_exit;
    mbedtls_mpi_init(tmp3);



    u = H_nn(usr->hash_alg, usr->A, B,1);

    if (!u)
       goto cleanup_and_exit;

    x = calculate_x( usr->hash_alg, s, usr->username, usr->password, usr->password_len );

    if (!x)
       goto cleanup_and_exit;

    k = H_nn(usr->hash_alg, usr->ng->N, usr->ng->g,1);

    if (!k)
       goto cleanup_and_exit;

    /* SRP-6a safety check */
    if( mbedtls_mpi_cmp_int( B, 0 ) != 0 && mbedtls_mpi_cmp_int( u, 0 ) !=0 )
    {
        mbedtls_mpi_exp_mod(v, usr->ng->g, x, usr->ng->N, RR);
        /* S = (B - k*(g^x)) ^ (a + ux) */
        mbedtls_mpi_mul_mpi( tmp1, u, x );
        mbedtls_mpi_mod_mpi( tmp1, tmp1, usr->ng->N);
        mbedtls_mpi_add_mpi( tmp2, usr->a, tmp1);
        mbedtls_mpi_mod_mpi( tmp2, tmp2, usr->ng->N);
        /* tmp2 = (a + ux)      */
        mbedtls_mpi_exp_mod( tmp1, usr->ng->g, x, usr->ng->N, RR);
        mbedtls_mpi_mul_mpi( tmp3, k, tmp1 );
        mbedtls_mpi_mod_mpi( tmp3, tmp3, usr->ng->N);
        /* tmp3 = k*(g^x)       */
        mbedtls_mpi_sub_mpi(tmp1, B, tmp3);
        /* tmp1 = (B - K*(g^x)) */
        mbedtls_mpi_exp_mod( usr->S, tmp1, tmp2, usr->ng->N, RR);

        hash_num(usr->hash_alg, usr->S, usr->session_key);

        calculate_M( usr->hash_alg, usr->ng, usr->M, usr->username, s, usr->A, B, usr->session_key );
        calculate_H_AMK( usr->hash_alg, usr->H_AMK, usr->A, usr->M, usr->session_key );

        *bytes_M = usr->M;
        *len_M = hash_length( usr->hash_alg );
        
    }
    else
    {
        *bytes_M = NULL;
        if (len_M) *len_M   = 0;
    }

 cleanup_and_exit:

    if (s) { mbedtls_mpi_free(s); free(s);}
    if (B) { mbedtls_mpi_free(B); free(B);}
    if (u) { mbedtls_mpi_free(u); free(u);}
    if (x) { mbedtls_mpi_free(x); free(x);}
    if (k) { mbedtls_mpi_free(k); free(k);}
    if (v) { mbedtls_mpi_free(v); free(v);}
    if (tmp1) { mbedtls_mpi_free(tmp1);free(tmp1);}
    if (tmp2) { mbedtls_mpi_free(tmp2);free(tmp2);}
    if (tmp3) { mbedtls_mpi_free(tmp3);free(tmp3);}
}


int srp_user_verify_session( SRPUser * usr, const unsigned char * bytes_HAMK )
{
    if ( memcmp( usr->H_AMK, bytes_HAMK, hash_length(usr->hash_alg) ) == 0 ) {
        usr->authenticated = 1;
		return 1;
	}
	return 0;
}