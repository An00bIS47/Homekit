#include <stdio.h>
#include <output_export.h>



#ifdef __GNUC__
void output_start(unsigned int baudrate __attribute__((unused)))
#else
void output_start(unsigned int baudrate)
#endif
{
    ;
}

void output_char(int c)
{
    putchar(c);
}

void output_flush(void)
{
    fflush(stdout);
}

void output_complete(void)
{
   ;
}