#include <stdio.h>

int foo(void) {
    printf("function call foo : OK\n");
    return 0;
}

int bar(void) {
    printf("function call bar : OK\n");
    return 0;
}

int func1(int x) {
    int ret = x+1;
    printf("function call func1(x = %d) = %d\n", x, ret);
    return ret;
}

int func2(int x, int y) {
    int ret = x+y;
    printf("function call func2(x = %d, y = %d) = %d\n", x, y, ret);
    return ret;
}

int func3(int x, int y, int z) {
    int ret = x+y+z;
    printf("function call func3(x = %d, y = %d, z = %d) = %d\n", x, y, z, ret);
    return ret;
}
