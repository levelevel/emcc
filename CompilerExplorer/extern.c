       void func(void);
extern void func(void);
static void func(void); //error
       int aaa;
extern int aaa;
static int aaa; //error
static int bbb;
       int bbb; //error
int main(void) {
    extern void func(void);
    static void func(void); //error
    void func(void);
    return 1;
}