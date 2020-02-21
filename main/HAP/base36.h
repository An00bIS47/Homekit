
#ifndef _BASE36_H
#define _BASE36_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *str_times(char *s, int base, int times, int carry);
char *str_base_convert(char *dest, int based, const char *src, int bases);
char *str16_to_str36(char *dest, const char *src);

#ifdef __cplusplus
}
#endif

#endif