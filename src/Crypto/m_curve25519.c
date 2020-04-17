#include <esp_log.h>


#include "m_curve25519.h"

#include "mbedtls/error.h"

#define TAG "CURVE25519"

int m_curve25519_key_generate(mbedtls_ecdh_context ctx_srv, mbedtls_ctr_drbg_context ctr_drbg, uint8_t public_key[], uint8_t private_key[])
{
    int ret = 1;

    mbedtls_entropy_context entropy;

    mbedtls_ecdh_init( &ctx_srv );
    mbedtls_ctr_drbg_init( &ctr_drbg );


    const char pers[] = "ecdh";
    /*
     * Initialize random number generation
     */
    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               sizeof pers ) ) != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );
    } 


    ret = mbedtls_ecp_group_load( &ctx_srv.grp, MBEDTLS_ECP_DP_CURVE25519 );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_ecp_group_load returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );
        return 1;
    }

    ret = mbedtls_ecdh_gen_public( &ctx_srv.grp, &ctx_srv.d, &ctx_srv.Q,
                                   mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_ecdh_gen_public returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );
        return 1;
    }

    ret = mbedtls_mpi_write_binary( &ctx_srv.Q.X, public_key, CURVE25519_KEY_LENGTH );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_mpi_write_binary public key returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );
        return 1;
    }

    m_reverse(public_key, CURVE25519_KEY_LENGTH);
    
    ret = mbedtls_mpi_write_binary( &ctx_srv.d, private_key, CURVE25519_KEY_LENGTH );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_mpi_write_binary private_key returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        mbedtls_entropy_free( &entropy );
        return 1;
    }

    m_reverse (private_key, CURVE25519_KEY_LENGTH);
    
    return 0;
}


int m_curve25519_shared_secret(mbedtls_ecdh_context ctx_srv, mbedtls_ctr_drbg_context ctr_drbg, uint8_t public_key[], 
        uint8_t* secret, int* secret_length)
{
    int ret = mbedtls_mpi_lset( &ctx_srv.Qp.Z, 1 );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_mpi_lset returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        return 1;
    }

    m_reverse (public_key, CURVE25519_KEY_LENGTH);
    ret = mbedtls_mpi_read_binary( &ctx_srv.Qp.X, public_key, CURVE25519_KEY_LENGTH );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_mpi_read_binary returned %d\n", ret );
        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        return 1;
    }

    ret = mbedtls_ecdh_compute_shared( &ctx_srv.grp, &ctx_srv.z,
                                       &ctx_srv.Qp, &ctx_srv.d,
                                       mbedtls_ctr_drbg_random, &ctr_drbg );
    if( ret != 0 )
    {
        ESP_LOGE( TAG, " failed\n  ! mbedtls_ecdh_compute_shared returned %d\n", ret );

        // char error[255];
        // int error_size = 0;
        // mbedtls_strerror(ret, error, error_size);
        // ESP_LOGE( TAG, "error: %s\n", error);

        mbedtls_ecdh_free( &ctx_srv );
        mbedtls_ctr_drbg_free( &ctr_drbg );
        return 1;
    }

    ret = mbedtls_mpi_write_binary(&ctx_srv.z, secret, sizeof(secret));
    m_reverse (secret, sizeof (secret));

    return 0;
}

void m_reverse (uint8_t* buf, size_t sz)
{
    uint8_t* last = buf + sz -1;
    uint8_t tmp;
    while (last > buf) {   //when last & buf meet in the middle we're done
        tmp = *buf;
        *buf = *last;
        *last = tmp;
        buf++;
        last--;
    }
}