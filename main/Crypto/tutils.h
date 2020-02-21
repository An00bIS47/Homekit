#pragma once
#include "mbedtls/bignum.h"
void tutils_array_print(const char* tag, const unsigned char* buf, int len);
void tutils_mpi_print(const char* tag, const mbedtls_mpi* x);