#ifdef _emcc
#include "builtin.h"
#endif

#include <stdio.h>
#include <stdarg.h>

void func(FILE*fp, char*fmt, ...) {
    va_list ap, ap2;
    va_start(ap, fmt);
    va_end(ap);

    puts(fmt);
    printf("&ap=%x\n", ap);
    vfprintf(fp, fmt, ap);
    int i = va_arg(ap, int);
    char *p = va_arg(ap, char*);
    printf("arg3=%d\narg4=%s\n", i, p);

    long l = va_arg(ap, long);
    p = va_arg(ap, char*);
    printf("arg5=%ld\narg6=%s\n", l, p);
    l = va_arg(ap, long);
    p = va_arg(ap, char*);
    printf("arg7=%ld\narg8=%s\n", l, p);
    //va_copy(ap2,ap);
}

int main() {
    func(stdout, "arg1=%d\narg2=%s\n", 1, "ARG2", 3, "ARG4", 5, "ARG6", 7, "ARG8");
    return 0;
}
