#include "gcc_def.h"
#include <stdio.h>
#include <ctype.h>
extern int ext_func(void);
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
