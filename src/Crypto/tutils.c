#include <stdlib.h>
#include <stdio.h>

#include "tutils.h"
void tutils_array_print(const char* tag, const unsigned char* buf, int len)
{
    printf("=== [%s] (len:%d) ===\n", tag, len);
	int need_lf=1;
    for (int i=0; i<len; i++) {
        if ((i & 0xf) == 0xf) {
	        printf("%02X\n", buf[i]);
			need_lf=0;
		} else {
	        printf("%02X ", buf[i]);
			need_lf=1;
		}
    }
    if (need_lf){
		printf("\n======\n");
	} else {
		printf("======\n");
	}
}

void tutils_mpi_print(const char* tag, const mbedtls_mpi* x)
{
    int len_x = mbedtls_mpi_size(x);
    unsigned char* num = malloc(len_x);
    mbedtls_mpi_write_binary(x, num, len_x);
	tutils_array_print(tag,num,len_x);
    free(num);
}