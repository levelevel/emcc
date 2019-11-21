//test_emcpp.c

#define MAC1
#define MAC2 mac2
"MAC2"=MAC2
#ifdef MAC1
int main (void) {
    int a=123, b=456;
    return (a+b);
}
#endif