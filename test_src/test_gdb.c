#include "gcc_def.h"
#include <stdio.h>
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
    int a = 1;
    a = func(a);
    return a;
}
