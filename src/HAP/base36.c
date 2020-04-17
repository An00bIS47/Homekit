#include "base36.h"

char *str_times(char *s, int base, int times, int carry) {
  size_t len = strlen(s);
  for (size_t i = len; i > 0;) {
    i--;
    int acc = strtol((char[2] ) { s[i], 0 }, 0, base) * times + carry;
    s[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[acc % base];
    carry = acc / base;
  }
  while (carry) {
    memmove(&s[1], &s[0], ++len);
    s[0] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[carry % base];
    carry /= base;
  }
  return s;
}

char *str_base_convert(char *dest, int based, const char *src, int bases) {
  strcpy(dest, "0");
  while (*src) {
    str_times(dest, based, bases, strtol((char [2]) {*src,0}, 0, bases));
    src++;
  }
  return dest;
}

char *str16_to_str36(char *dest, const char *src) {
  return str_base_convert(dest, 36, src, 16);
}
