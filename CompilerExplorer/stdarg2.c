#include <stdio.h>
#include <stdarg.h>

void func(FILE*fp, char*fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_end(ap);

    puts(fmt);
    printf("%x", ap);
    int p1 = va_arg(ap, int);
    long p2 = va_arg(ap, long);
    vfprintf(fp, fmt, ap);
    va_copy(ap2,ap);
}
