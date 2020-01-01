#include "gcc_def.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
extern int ext_func(void);
    static char *fa_argv[] = {"abc", "def", "pqy", "xyz"};
static int funcarg1_p2(int argc, char *argv[]) {
    for (int i=0; i<argc-1; argv++, i++) {
        if (strcmp( argv[1], fa_argv[i+1])) return 0;
    }
    return 1;
}
int func(int aaa) {
    int bbb = 1;
    for (int idx=0;idx<3;idx++) {
        aaa += bbb;
    }
    printf("aaa=%d", aaa);
    return aaa;
}
int main() {
    printf("isdigit=%d\n", isdigit('1'));
    int a = 1;
    a = func(a);
    return a;
}
