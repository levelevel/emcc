#ifdef _9cc
//#define void
#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))
#define size_t long
#define NULL 0
int printf(const char *format, ...);
void exit(int status);
char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#endif

#define GLOBAL
#define TEST(f) if(!f()) {printf("Error at %s:%d:%s\n",__FILE__,__LINE__,#f);exit(1);} else {printf("  OK: %s\n",#f);}

static int f42() {42; ;;;;;}
static int addsub1(){
    int a=10, b=3, c=a+b*(-2), d=a/b, e=a%b;
    return c==4 && d==3 && e==1;
}
static char addsub1c(){
    char a=10, b=3, c=a+b*(-2), d=a/b, e=a%b;
    return c==4 && d==3 && e==1;
}
static short addsub1s(){
    short a=10, b=3, c=a+b*(-2), d=a/b, e=a%b;
    return c==4 && d==3 && e==1;
}
static long addsub1l(){
    long a=10, b=3, c=a+b*(-2), d=a/b, e=a%b;
    return c==4 && d==3 && e==1;
}
static long long addsub1ll(){
    long long a=10, b=3, c=a+b*(-2), d=a/b, e=a%b;
    return c==4 && d==3 && e==1;
}
static int addsub_mix1() {
    char c = 0x7f; unsigned char uc = 0x7f; int ic = 0x7f;
    short s = 0x7fff; unsigned short us = 0x7fff; int is = 0x7fff;
    int i = 0x7fffffff; unsigned int ui = 0x7fffffff; long li = 0x7fffffff;
    long l = 0x7fffffffffffffff; unsigned long ul = 0x7fffffffffffffff;
    return
        c+uc == uc*2 && ic+c == uc*2 &&
        s+us == us*2 && is+s == us*2 &&
        i+ui == ui*2 && li+i == ui*2 &&
        l+ul == ul*2;
}
static int addsub_mix2() {
    char  c = -1; unsigned char  uc = 1;
    short s = -1; unsigned short us = 1;
    int   i = -1; unsigned int   ui = 1;
    long  l = -1; unsigned long  ul = 1;
    return
        uc - c == 2 && uc == -c &&
        us - s == 2 && us == -s &&
        ui - i == 2 && ui == -i &&
        ul - l == 2 && ul == -l &&
        s == c && c == s && us == uc && us == -c &&
        i == c && c == i && ui == uc && ui == -c &&
        i == s && s == i && ui == us && ui == -s &&
        l == c && c == l && ul == uc && ul == -c &&
        l == s && s == l && ul == us && ul == -s &&
        l == i && i == l && ul == ui && ul == -i;
}
static int addsub() {
    TEST(addsub1);
    TEST(addsub1c);
    TEST(addsub1s);
    TEST(addsub1l);
    TEST(addsub1ll);
    TEST(addsub_mix1);
    TEST(addsub_mix2);
    return 
#ifdef _9cc
        42 == f42() &&    //これはCの仕様とは異なる
#endif
        14 == 10 + 2 * 3 - 4/2 &&
        3  == (((2+4)*1)/2) &&
        2  == +5%-(-3) &&
        (1,2,3)==3 &&
        (8|7) == 15 &&
        (8^9) == 1 &&
        (15&3) == 3 &&
        ~0 == -1 &&
        8>>2==2 &&
        8<<2==32;
}

static int eq_rel1() {
    return
        (5 == 5) == 1 &&
        (6 != 7) == 1 &&
        (1 <  2) == 1 &&
        (2 >  1) == 1 &&
        (3 <= 3) == 1 &&
        (4 >= 4) == 1;
}
static int eq_rel2() {
    return
        (5 == 4) == 0 &&
        (6 != 6) == 0 &&
        (2 <  1) == 0 &&
        (1 >  2) == 0 &&
        (4 <= 3) == 0 &&
        (3 >= 4) == 0;
}
static int eq_rel() {
    TEST(eq_rel1);
    TEST(eq_rel2);
    return 1;
}

static int while1() {
    int a=3;
    while (a) a = a-1; 
    while (1) {break;}
    while(0);
    int i=0, sum=0;
    while (i<=10) {sum = sum + i; i++;}
    return a==0 && sum==55;
}

static int do1() {
    int a=3;
    do a--; while (a); 
    do {break;} while (1);
    do ; while(0);
    int i=0, sum=0;
    do {sum += i; i++;} while (i<=10);
    return a==0 && sum==55;
}

static int for1() {
    int a, b, i, j;
    for (i=0; i<5; i++) a=i;
    for (i=10; i; i--) {b=i; continue; b=999;};
    //for (int i=10; i; i--) {};
    for (;;) {break;}

    int x=0;
    for (i=0;;i++) {
        if (i>=3) { x++; break ;}
        for (j=0;;j++) {
            if (j>=5)  { break; }
        }
    }
    return a==4 && b==1;
}
static int for2() {
    int i=5;
    int n1=i, n2, n3;
    for (int i=0;i<=10;i++) {
        n2 = i;
        int i = 20;
        n3 = i;
    }
    return n1==5 && n2==10 && n3==20;
}

static int gtri_x;
static int gtri_a = 1?2:gtri_x;
static int tri_cond() {
    int a,b;
    a = 1?10:20;
    b = 0?10:20;
    return
        a==10 && b==20 && gtri_a==2;
}
static int goto1(void) {
    int i=0, L1=2;
    goto L1;
    i=1;
    {
      L1:i++;
      if (i>=10) goto L2;
      goto L1;
    }
    L2: return i==10;
}
static int iterate() {
    TEST(while1);
    TEST(do1);
    TEST(for1);
    TEST(tri_cond);
    TEST(goto1);
    return 1;
}

static int if1() {
    if(0);
    if(0) while(0) if(1) while(0);

    int sum1 = 0;
    for (int i=10;i>0;i--) {sum1=sum1+i;}

    int d=0; if(0) d=1; else d=2;
    int e=0; if(0) {e=1;} else if (1) {e=2;} else {e=3;}

    return d==2 && e==2 && sum1==55;
}
static int case1() {
    int x, y, z;
    switch (1) {
    case 1: x=1;
        switch ('b') {
        case 'a': x=11; break;
        case 'b': y=12;
            switch (3) {
            case 1: x=11; break;
            case 2: y=12; break;
            default: z=23; break;
            }
            break;
        }
        break;
    case 2: y=2; break;
    }
    return x==1 && y==12 && z==23;
}
static int selection(void) {
    TEST(if1);
    TEST(case1);
    return 1;
}

static int inc() {
    int a=1, b, c, x[]={1,2,3,4}, *px=&x[0];
    b = a++;
    c = ++b;
    x[1]++; x[2]+=10;
    px++; px+=2;
    return a+b+c==6 && x[1]==3 && x[2]==13 && *px==4;
}
static int inc_char() {
    char a=1, b, c, x[]={1,2,3,4}, *px=&x[0];
    b = a++;
    c = ++b;
    x[1]++; x[2]+=10;
    px++; px+=2;
    return a+b+c==6 && x[1]==3 && x[2]==13 && *px==4;
}
static int inc_short() {
    short a=1, b, c, x[]={1,2,3,4}, *px=&x[0];
    b = a++;
    c = ++b;
    x[1]++; x[2]+=10;
    px++; px+=2;
    return a+b+c==6 && x[1]==3 && x[2]==13 && *px==4;
}
static int inc_long() {
    long a=1, b, c, x[]={1,2,3,4}, *px=&x[0];
    b = a++;
    c = ++b;
    x[1]++; x[2]+=10;
    px++; px+=2;
    return a+b+c==6 && x[1]==3 && x[2]==13 && *px==4;
}
static int dec() {
    int a=2, b, c, x[]={1,2,3,4}, *px=&x[3];
    b = a--;
    c = --b;
    x[1]--; x[2]-=10;
    px--; px-=2;
    return a+b+c==3 && x[1]==1 && x[2]==-7 && *px==1;
}
static int dec_char() {
    char a=2, b, c, x[]={1,2,3,4}, *px=&x[3];
    b = a--;
    c = --b;
    x[1]--; x[2]-=10;
    px--; px-=2;
    return a+b+c==3 && x[1]==1 && x[2]==-7 && *px==1;
}
static int dec_short() {
    char a=2, b, c, x[]={1,2,3,4}, *px=&x[3];
    b = a--;
    c = --b;
    x[1]--; x[2]-=10;
    px--; px-=2;
    return a+b+c==3 && x[1]==1 && x[2]==-7 && *px==1;
}
static int dec_long() {
    long a=2, b, c, x[]={1,2,3,4}, *px=&x[3];
    b = a--;
    c = --b;
    x[1]--; x[2]-=10;
    px--; px-=2;
    return a+b+c==3 && x[1]==1 && x[2]==-7 && *px==1;
}
static int incdec() {
    TEST(inc);
    TEST(inc_char);
    TEST(inc_short);
    TEST(inc_long);
    TEST(dec);
    TEST(dec_char);
    TEST(dec_short);
    TEST(dec_long);
    return 1;
}

static int add(int a, int b){return a+b;}
static int add3(int a, int b, int c){return a+b+c;}
static int fact(int a) {
    if (a==0) return 0;
    else return a + fact(a-1);
}
static int fib(int a) {
    if (a==0) return 0;
    else if (a==1) return 1;
    else return fib(a-1) + fib(a-2);
}
static char* stdarg(char fmt, ...) {
#ifdef _9cc
    #define va_start __va_start
    #define va_list char*
#endif
    static char buf[256];
#if 0
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
#endif
    return buf;
}

static void void_func(void);
static void void_func(void) {
    ;
}
static void *void_funcp(void*);
static void *void_funcp(void*p) {
    return p;
}

       short funcdecl1s(short s);
static char *funcdecl1c(char *cp);
extern long funcdecl1l(long l, char*cp, ...); 
static int funcdecl1();
static int funcdecl1(void);
static int funcdecl1() {
    int funcdecl1();
    int funcdelc1i(int i);
    int funcdelc1cp(char *cp);
    int funcdelc1icp(int i, char*cp);
    return 1;
}

       short funcdecl2s(short);
static char *funcdecl2c(char);
extern long funcdecl2l(long, char*, ...); 
static int funcdecl2();
static int funcdecl2(void);
static int funcdecl2(void) {
    int funcdecl2();
    int funcdelc2i(int);
    int funcdelc2cp(char);
    int funcdelc2icp(int, char*);
    return 1;
}

static int funcdecl3() {
    fd3_func(); //未定義の関数
    return 1;
}
int fd3_func() {
    return 1;
}

static int funcdecl4a(int a, char*p);
static int funcdecl4a(int, char*);
static int funcdecl4a();
static int funcdecl4a(int a, char*p) {
    return 1;
}
static int funcdecl4b(char*fmp, ...);
static int funcdecl4b(char*, ...);
//static int funcdecl4b();  //GCCではこれはエラーになる
static int funcdecl4b(char*fmt, ...) {
    return 1;
}

static int fp1_add(int a, int b) { return a+b; }
static int (*fp1_fp)(int, int)=fp1_add;
static int funcp1(void) {
    int (*fp)(int, int);
    fp = fp1_add;
    return fp(1,2)==3 && fp1_fp(2,3)==5;
}

extern long fp2_add(long a, long b);    //実体はextern.cで定義
static int (*fp2_fp)(long, long)=&fp2_add;
static int funcp2(void) {
    long (*fp)(long, long);
    fp = fp2_add;
    return fp(11,12)==23 && fp2_fp(2,3)==5;
}

static int *fp3_inc(int *a){(*a)++; return a;}    //*aを++して、aを返す
static int funcp3(void) {
    int *(*fp)(int *a) = &fp3_inc;
    int x=1;
    return *fp(&x)==2 && x==2;
}

static char *fp4_str(void){static char str[]="ABC"; return str;}
static int funcp4(void) {
    char *(*fp)(void) = fp4_str;
    return
        strcmp(fp(),"ABC")==0 &&
        *fp()=='A' && fp()[1]=='B' && fp4_str()[2]=='C';
}

static int fp5_func(void){return 10;}
static int funcp5(void) {
    int (*fp)(void) = fp5_func;
    int (**fpp)(void) = &fp;
    return
        fp()==10 && 
        (fp)()==10 &&
        //(*fpp)()==10 &&
        1;
}

static int func() {
    void_func();
    TEST(funcdecl1);
    TEST(funcdecl2);
    TEST(funcdecl3);
    TEST(funcp1);
    TEST(funcp2);
    TEST(funcp3);
    TEST(funcp4);
    TEST(funcp5);
    //int (**x)[2];
    //int (**x)();
    //int *(*x)[];
    //int (*x)[][];
    //int (*x[])[];
    //int (*x[])();
    //int (*x())[];
    //int (*x())();
    return
        fact(10) == 55 &&
        fib(10) == 55;
}

static int pf1(int *p, int**pp) {
    return *p + **pp;
}
static int pointer1() {
    int  a = 1;
    int *p = &a;
    return *p == 1 && pf1(&a, &p)==2;
}
static int  gp1_a = 1;
static int *gp1_p = &gp1_a;
static int pointer1g() {
    return *gp1_p == 1 && pf1(&gp1_a, &gp1_p)==2;
}
static int pointer2() {
    int a;
    int *p=&a;
    *p=12;
    return a==12;
}
static int  gp2_a;
static int *gp2_p=&gp2_a;
static int pointer2g() {
    *gp2_p=12;
    return gp2_a==12;
}
static int**pointer3(int ***********a) {
    &(a);
    &(*(a));
    &**(**a);
    return 0;
}
static int* pf4(int *a){return a;}
static int pointer4() {
    int x;
    int *y=pf4(&x);
    return &x==y;
}
static int pointer5() {
    int **p=0;
    p++;
    ++p;
    return (long)(p-1) == sizeof(int*);
}
static int pointer6() {
    int *p=0;
    int a=2;
    p = p+a;
    p = p+1;
    p--;
    --p;
    return (long)(1+p) == sizeof(int)*2;
}
static int pointer7() {
    int a, *p=&a, *q=&a+5;
    return p+5==q && q-p==5;
}
static int pointer() {
    TEST(pointer1);
    TEST(pointer1g);
    TEST(pointer2);
    TEST(pointer2g);
    TEST(pointer4);
    TEST(pointer5);
    TEST(pointer6);
    TEST(pointer7);
    return 1;
}

static int array1() {
    int a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        (long)a==(long)&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
static int array1c() {
    char a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        (long)a==(long)&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
static int array1s() {
    short a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        (long)a==(long)&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
static int array1l() {
    long a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        (long)a==(long)&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
static int af2(int *a){return a[0]+a[1]+a[2];}
static int array2() {
    int a[4]={1,2,3,4}, *p=a, *q=&a[2];
    return af2(a)==6 && p[1]==2, *q==3;
}
GLOBAL int a2g_a[4]={1,2,3,4};
static int a2g_b[4]={1,2,3,4}, *a2g_p=&a2g_a, *a2g_q=&a2g_b[2];
static int array2g() {
    return af2(a2g_a)==6 && a2g_p[1]==2, *a2g_q==3;
}
static char af2c(char *a){return a[0]+a[1]+a[2];}
static int array2c() {
    char a[4]={1,2,3,4}, *p=a, *q=&a[2];
    return af2c(a)==6 && p[1]==2, *q==3;
}
static short af2s(short *a){return a[0]+a[1]+a[2];}
static int array2s() {
    short a[4]={1,2,3,4}, *p=a, *q=&a[2];
    return af2s(a)==6 && p[1]==2, *q==3;
}
static long af2l(long *a){return a[0]+a[1]+a[2];}
static int array2l() {
    long a[4]={1,2,3,4}, *p=a, *q=&a[2];
    return af2l(a)==6 && p[1]==2, *q==3;
}
static int array3() {
    int a[4][5], *p=(int*)a, b[4][5][6], *q=(int*)b;
    a[2][3] = 10;
    b[2][3][4] = 20;
    return
        (long)a[1] == (long)(a+1) && p[2*5+3]==10 && *(q+2*5*6+3*6+4)==20;
}
static int array3c() {
    char a[4][5], *p=(char*)a, b[4][5][6], *q=(char*)b;
    a[2][3] = 10;
    b[2][3][4] = 20;
    return
        (long)a[1] == (long)(a+1) && p[2*5+3]==10 && *(q+2*5*6+3*6+4)==20;
}
static int array3s() {
    short a[4][5], *p=(short*)a, b[4][5][6], *q=(short*)b;
    a[2][3] = 10;
    b[2][3][4] = 20;
    return
        (long)a[1] == (long)(a+1) && p[2*5+3]==10 && *(q+2*5*6+3*6+4)==20;
}
static int array3l() {
    long a[4][5], *p=(long*)a, b[4][5][6], *q=(long*)b;
    a[2][3] = 10;
    b[2][3][4] = 20;
    return
        (long)a[1] == (long)(a+1) && p[2*5+3]==10 && *(q+2*5*6+3*6+4)==20;
}
static int sarray2() {
//    static int a[4]={1,2,3,4}, *p=a, *q=&a[2];
//    return af2(a)==6 && p[1]==2, *q==3;
}
static int array() {
    TEST(array1);
    TEST(array1c);
    TEST(array1s);
    TEST(array1l);
    TEST(array2);
    TEST(array2g);
    TEST(array2c);
    TEST(array2s);
    TEST(array2l);
    TEST(array3);
    TEST(array3c);
    TEST(array3s);
    TEST(array3l);

//    TEST(sarray2);
    return 1;
}

GLOBAL char sg1_buf[20];
GLOBAL char*sg1_p[4];
GLOBAL char sg1_str1[5] = "ABC";
GLOBAL char sg1_str2[ ] = "ABC";
GLOBAL char sg1_str3[4] = "ABCDE"; //長すぎる
static int string1() {
    char buf[20];
    strcpy(buf, "abc");

    char*p[4];
    p[0] = 0;
    p[1] = "ABCD";

    char str1[5] = "ABC";
    char str2[ ] = "ABC";
    char str3[4] = "ABCDE"; //長すぎる

    return
        strcmp(buf, "abc")==0 &&
        *(p[1]+2)-'A'==2 &&
        strcmp(str1, "ABC")==0 && strlen(str1)==3 &&
        strcmp(str2, "ABC")==0 && strlen(str2)==3 &&
        strncmp(str3, "ABCD", 4)==0 &&
        strcmp(str1, sg1_str1)==0;
}
static int string1g() {
    strcpy(sg1_buf, "abc");

    sg1_p[0] = 0;
    sg1_p[1] = "ABCD";

    return
        strcmp(sg1_buf, "abc")==0 &&
        *(sg1_p[1]+2)-'A'==2 &&
        strcmp(sg1_str1, "ABC")==0 && strlen(sg1_str1)==3 &&
        strcmp(sg1_str2, "ABC")==0 && strlen(sg1_str2)==3 &&
        strncmp(sg1_str3, "ABCD", 4)==0;
}
static int string() {
    TEST(string1);
    TEST(string1g);
    return 1;
}

static int init1() {
    int a[] = {1,2,1+2};
    int b = {5,};
    char*p    = "ABC";
    char s1[] = "ABC";
    char s2[] = {'A', 66, 'A'+2, 0};
    int i=1;
    char ac[]={i,i*2,10};
    int  ai[]={i,i*2,10};

    return
        a[0]+a[1]+a[2]==6 && sizeof(a)==sizeof(int)*3 &&
        b==5 && 
        strcmp(p,s1)==0 &&
        strcmp(s1,s2)==0 &&
        ac[0]+ac[1]+ac[2]==13 &&
        ai[0]+ai[1]+ai[2]==13;
}

GLOBAL int i1g_a[] = {1,2,1+2};
GLOBAL int i1g_b = {5,};
GLOBAL char*i1g_p    = "ABC";
GLOBAL char i1g_s1[] = "ABC";
GLOBAL char i1g_s2[] = {'A', 66, 'A'+2, 0};
static int init1g() {
    return
        i1g_a[0]+i1g_a[1]+i1g_a[2]==6 && sizeof(i1g_a)==sizeof(int)*3 &&
        i1g_b==5 && 
        i1g_p[0]=='A' &&
        strcmp(i1g_p,i1g_s1)==0 &&
        strcmp(i1g_s1,i1g_s2)==0;
}
static int i1sg_a[] = {1,2,1+2};
static int i1sg_b = {5,};
static char*i1sg_p    = "ABC";
static char i1sg_s1[] = "ABC";
static char i1sg_s2[] = {'A', 66, 'A'+2, 0};
static int init1sg() {
    return
        i1sg_a[0]+i1sg_a[1]+i1sg_a[2]==6 && sizeof(i1sg_a)==sizeof(int)*3 &&
        i1sg_b==5 && 
        i1sg_p[0]=='A' &&
        strcmp(i1sg_p,i1sg_s1)==0 &&
        strcmp(i1sg_s1,i1sg_s2)==0;
}
GLOBAL int  i2g_x, *i2g_p = 2 + &i2g_x - 1;
GLOBAL char i2g_c, *i2g_ac[3] = {0,&i2g_c+1,(char*)3,(char*)4};    //初期値多い
GLOBAL int  i2g_i, *i2g_ai[5] = {0,&i2g_i+1,(int*) 3,(int*) 4};    //初期値足りない分は0
static int init2g() {
    return 
        &i2g_x+1 == i2g_p && i2g_x==0 &&
        i2g_ac[1]==&i2g_c+1 && i2g_ac[2]==(char*)3 &&
        i2g_ai[1]==&i2g_i+1 && i2g_ai[2]==(int*)3 && i2g_ai[4]==0;
}
static int i3g_a=9, *i3g_p=&*&i3g_a;
static int init3g() {
    return *i3g_p==9;
}
static int init() {
    TEST(init1);
    TEST(init1g);
    TEST(init1sg);
    TEST(init2g);
    TEST(init3g);
    return 1;
}

static int align1() {
    char c1; int i; char c2; int*p;
    unsigned long ui = (unsigned long)&i, up = (unsigned long)&p;
    return ui%4==0 && up%8==0;
}
static char ag1_c1;
static int  ag1_i;
static char ag1_c2;
static char*ag1_p;
static int align1g() {
    unsigned long ui = (unsigned long)&ag1_i, up = (unsigned long)&ag1_p;
    return ui%4==0 && up%8==0;
}
static int align() {
    TEST(align1);
    TEST(align1g);
    return 1;
}

static int size_of1() {
    int n, *p, a[2*4], a2[2][3];
    typedef int INT;
    typedef unsigned int* UINTP;
    typedef const INT INTA3[3];
    typedef int INTA[];
    INTA A4={10,20,30,40};
    typedef int INTAA2[][2];
//    INTAA2 A3A2={{00,01},{10,11},{20,21}};
    return
        sizeof(n)==4 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==4*8 && sizeof(a[0])==4 &&
        sizeof(a2)==4*2*3 && sizeof(a2[0])==4*3 && sizeof(a2[0][1])==4 &&
        sizeof(int)==4 && sizeof(int*)==8 && sizeof(int(*(*)))==8 &&
        sizeof(INT)==4 && sizeof(UINTP)==8 && sizeof(INTA3)==4*3 && sizeof(A4)==4*4 &&
        sizeof(unsigned int)==4 && sizeof(signed int)==4 &&
        sizeof(int[5])==4*5 && sizeof(int*[3])==8*3 &&
        sizeof(int[5][2])==4*5*2 && sizeof(int*[3][2])==8*3*2 &&
        sizeof(1)==4 && sizeof(1==1)==4 && sizeof(n=1)==4;
}
static int size_of1c() {
    char n, *p, a[2*4], a2[2][3];
    typedef char CHAR;
    typedef unsigned char* UCHARP;
    typedef const CHAR CHARA3[3];
    typedef char CHARA[];
    CHARA A4={10,20,30,40};
    return
        sizeof(n)==1 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==1*8 && sizeof(a[0])==1 &&
        sizeof(a2)==1*2*3 && sizeof(a2[0])==1*3 && sizeof(a2[0][1])==1 &&
        sizeof(char)==1 && sizeof(char*)==8 && sizeof(char(*(*)))==8 &&
        sizeof(CHAR)==1 && sizeof(UCHARP)==8 && sizeof(CHARA3)==1*3 && sizeof(A4)==1*4 &&
        sizeof(unsigned char)==1 && sizeof(signed char)==1 &&
        sizeof(char[5])==1*5 && sizeof(char*[3])==8*3 &&
        sizeof(char[5][2])==1*5*2 && sizeof(char*[3][2])==8*3*2 &&
        sizeof(1)==4 && sizeof(1==1)==4 && sizeof(n=1)==1;
}
static int size_of1s() {
    short n, *p, a[2*4], a2[2][3];
    typedef short SHORT;
    typedef unsigned short* USHORTP;
    typedef const SHORT SHORTA3[3];
    return
        sizeof(n)==2 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==2*8 && sizeof(a[0])==2 &&
        sizeof(a2)==2*2*3 && sizeof(a2[0])==2*3 && sizeof(a2[0][1])==2 &&
        sizeof(short)==2 && sizeof(short*)==8 && sizeof(short(*(*)))==8 &&
        sizeof(SHORT)==2 && sizeof(USHORTP)==8 && sizeof(SHORTA3)==2*3 &&
        sizeof(unsigned short)==2 && sizeof(signed short)==2 &&
        sizeof(short[5])==2*5 && sizeof(short*[3])==8*3 &&
        sizeof(short[5][2])==2*5*2 && sizeof(short*[3][2])==8*3*2 &&
        sizeof(1)==4 && sizeof(1==1)==4 && sizeof(n=1)==2;
}
static int size_of1l() {
    long n, *p, a[2*4], a2[2][3];
    typedef long LONG;
    typedef unsigned long* ULONGP;
    typedef const LONG LONGA3[3];
    return 
        sizeof(n)==8 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==8*8 && sizeof(a[0])==8 &&
        sizeof(a2)==8*2*3 && sizeof(a2[0])==8*3 && sizeof(a2[0][1])==8 &&
        sizeof(long)==8 && sizeof(long*)==8 && sizeof(long(*(*)))==8 &&
        sizeof(LONG)==8 && sizeof(ULONGP)==8 && sizeof(LONGA3)==8*3 &&
        sizeof(unsigned long)==8 && sizeof(signed long)==8 &&
        sizeof(long[5])==8*5 && sizeof(long*[3])==8*3 &&
        sizeof(long[5][2])==8*5*2 && sizeof(long*[3][2])==8*3*2 &&
        sizeof(1)==4 && sizeof(1==1)==4 && sizeof(n=1)==8;
}
static int size_of1ll() {
    long long n, *p, a[2*4], a2[2][3];
    typedef long long LLONG;
    typedef unsigned long long* ULLONGP;
    typedef const LLONG LLONGA3[3];
    return 
        sizeof(n)==8 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==8*8 && sizeof(a[0])==8 &&
        sizeof(a2)==8*2*3 && sizeof(a2[0])==8*3 && sizeof(a2[0][1])==8 &&
        sizeof(long long)==8 && sizeof(long long*)==8 &&
        sizeof(unsigned long long)==8 && sizeof(signed long long)==8 &&
        sizeof(LLONG)==8 && sizeof(ULLONGP)==8 && sizeof(LLONGA3)==8*3 &&
        sizeof(long long[5])==8*5 && sizeof(long long*[3])==8*3 &&
        sizeof(long long[5][2])==8*5*2 && sizeof(long long*[3][2])==8*3*2 &&
        sizeof(1)==4 && sizeof(1==1)==4 && sizeof(n=1)==8;
}
static int size_of1v() {
    void *p, *a[2*4], *a2[2][3];
    typedef void VOID;
    typedef void* VOIDP;
    typedef const VOIDP VOIDA3[3];
    return
        sizeof(p)==8 &&
        sizeof(a)==8*8 && sizeof(a[0])==8 &&
        sizeof(a2)==8*2*3 && sizeof(a2[0])==8*3 && sizeof(a2[0][1])==8 &&
        sizeof(void)==1 && sizeof(void*)==8 && sizeof(void(*(*)))==8 &&
        sizeof(VOID)==1 && sizeof(VOIDP)==8 && sizeof(VOIDA3)==8*3 &&
        sizeof(char*[3])==8*3 &&
        sizeof(char*[5][2])==8*5*2 && sizeof(char*[3][2])==8*3*2;
}
static int size_of1C() {
    const int n, *p, a[2*4], a2[2][3];
    typedef const int INT;
    typedef const unsigned int* UINTP;
    typedef const INT INTA3[3];
    return
        sizeof(n)==4 && sizeof(&n)==8 && sizeof(p)==8 &&
        sizeof(a)==4*8 && sizeof(a[0])==4 &&
        sizeof(a2)==4*2*3 && sizeof(a2[0])==4*3 && sizeof(a2[0][1])==4 &&
        sizeof(const int)==4 && sizeof(const int*)==8 && sizeof(const int(*(*)))==8 &&
        sizeof(INT)==4 && sizeof(UINTP)==8 && sizeof(INTA3)==4*3 && 
        sizeof(unsigned const int)==4 && sizeof(const signed int)==4 &&
        sizeof(const int[5])==4*5 && sizeof(const int*[3])==8*3 &&
        sizeof(const int[5][2])==4*5*2 && sizeof(int const*[3][2])==8*3*2;
}
static int size_of2() {
    return
        _Alignof(char)==1            && _Alignof(char*)==8 &&
        _Alignof(char[5])==1         && _Alignof(char*[3])==8 &&
        _Alignof(char[5][2])==1      && _Alignof(char*[3][2])==8 &&
        _Alignof(short)==2           && _Alignof(short*)==8 &&
        _Alignof(short[5])==2        && _Alignof(short*[3])==8 &&
        _Alignof(short[5][2])==2     && _Alignof(short*[3][2])==8 &&
        _Alignof(int)==4             && _Alignof(int*)==8 &&
        _Alignof(int[5])==4          && _Alignof(int*[3])==8 &&
        _Alignof(int[5][2])==4       && _Alignof(int*[3][2])==8 &&
        _Alignof(long)==8            && _Alignof(long*)==8 &&
        _Alignof(long[5])==8         && _Alignof(long*[3])==8;
        _Alignof(long[5][2])==8      && _Alignof(long*[3][2])==8;
        _Alignof(long long)==8       && _Alignof(long long*)==8 &&
        _Alignof(long long[5][2])==8 && _Alignof(long long*[3][2])==8;
}
static int size_of() {
    TEST(size_of1);
    TEST(size_of1c);
    TEST(size_of1s);
    TEST(size_of1l);
    TEST(size_of1ll);
    TEST(size_of1v);
    TEST(size_of1C);
    TEST(size_of2);
    return 1;
}

static int type_of() {
    char  c; typeof(c) c2;
    short s; typeof(s) s2;
    int   i; typeof(i) i2;
    long  l; typeof(l) l2;
    long long ll; typeof(ll) ll2;
    char *p; typeof(p) p2;
    static int si;
    return
        sizeof(c) == sizeof(c2) &&
        sizeof(s) == sizeof(s2) &&
        sizeof(i) == sizeof(i2) &&
        sizeof(l) == sizeof(l2) &&
        sizeof(ll) == sizeof(ll2) &&
        sizeof(p) == sizeof(p2) &&
        sizeof(typeof(si))==4;
}

static int sc_x=0, sc_y=0;
static int scope1(void) {
    int sc_x; 
    sc_x=1;
    sc_y=2;
    return sc_x+sc_y==3;
}
static int scope2(void) {
    int a=1, a2;
    static int sa=2, sa2;
    if (1) {
        int a; a=10;
        static int sa; sa=20;
        a2 = a;
        sa2 = sa;
    }
    return
        a==1 && sa==2 && a2==10 && sa2==20;
}
static int scope3(void) {
    int a=1, a2;
    static int sa=2, sa2;
    {
        int a; a=10;
        static int sa; sa=20;
        a2 = a;
        sa2 = sa;
        {
            int a=100;
            static int sa; sa=200;
            a2 = a;
            sa2 = sa;
        }
    }
    return
        a==1 && sa==2 && a2==100 && sa2==200;
}
static int scope(void) {
    TEST(scope1);
    TEST(scope2);
    TEST(scope3);
    return 1;
}

static int overflow1() {
    int a = 0xffffffff;
    int b = 0 - 1;
    unsigned int ua = 0xffffffff;
    unsigned int ub = 0 - 1;
    return
        a == -1 && a<0 &&
        b == -1 && b<0 &&
        ua == -1 && ua > 0 && 
        ub == -1 && ub > 0  &&
        ua == ub && a == ub ; 
}
static int overflow1c() {
    char a = 0xff;
    char b = 0 - 1;
    unsigned char ua = 0xff;
    unsigned char ub = 0 - 1;
    return
        a == -1 && a<0 &&
        b == -1 && b<0 &&
        ua != -1 && ua > 0 && 
        ub != -1 && ub > 0  &&
        ua == ub && a != ub ; 
}
static int overflow1s() {
    short a = 0xffff;
    short b = 0 - 1;
    unsigned short ua = 0xffff;
    unsigned short ub = 0 - 1;
    return
        a == -1 && a<0 &&
        b == -1 && b<0 &&
        ua != -1 && ua > 0 && 
        ub != -1 && ub > 0  &&
        ua == ub && a != ub ; 
}
static int overflow1l() {
    long a = 0xffffffffffffffff;
    long b = 0 - 1;
    unsigned long ua = 0xffffffffffffffff;
    unsigned long ub = 0 - 1;
    return
        a == -1 && a<0 &&
        b == -1 && b<0 &&
        ua == -1 && ua > 0 && 
        ub == -1 && ub > 0  &&
        ua == ub && a == ub ; 
}
static int overflow1ll() {
    long long a = 0xffffffffffffffff;
    long long b = 0 - 1;
    unsigned long long ua = 0xffffffffffffffff;
    unsigned long long ub = 0 - 1;
    return
        a == -1 && a<0 &&
        b == -1 && b<0 &&
        ua == -1 && ua > 0 && 
        ub == -1 && ub > 0  &&
        ua == ub && a == ub ; 
}
static int overflow2() {
    char          c = -1;
    short int     s = -1;
    int           i = -1;
    long int      l = -1;
    long long int ll = -1;
    return
        c == s && c == i && c == l && c == ll &&
        s == i && s == l && s == ll &&
        i == l && i == ll &&
        l == ll; 
}
static int overflow() {
    TEST(overflow1);
    TEST(overflow1c);
    TEST(overflow1s);
    TEST(overflow1l);
    TEST(overflow1ll);
    TEST(overflow2);
    return 1;
}

static int integer_def() {
    short int si;
    long int li;
    long long int lli;
    signed char Sc;
    signed short int Ssi;
    signed int Si;
    signed long int Sli;
    signed long long int Slli;
    unsigned char uc;
    unsigned short int usi;
    unsigned int ui;
    unsigned long int uli;
    unsigned long long int ulli;
    signed S;
    unsigned u;

    int short is;
    int long il;
    int long long ill;
    long int long lil;
    long signed int long lSil;
    long int long unsigned lilu;
    return 1;
}

// extern
// - 関数外の変数、関数：宣言しない
// - 関数内の変数：宣言しない。スタック確保しない。
// - 初期化はできない。
// static
// - 関数外の変数、関数：global宣言しない
// - 関数内の変数：基本はグローバル変数と同じだが、ローカルスコープなので
//                名前が重複しないようにする(%s.%03d)。スタック確保しない。
extern char  g_extern_c;
extern short g_extern_s;
extern int   g_extern_i;
extern long  g_extern_l;

static char  g_static_c=2;
static short g_static_s=3;
static int   g_static_i=4;
static long  g_static_l=5;

static int extern1() {
    g_extern_s = 5;
    return
        g_extern_c==1 && g_extern_s==5 && g_extern_i==3 && g_extern_l==4 &&
        g_static_c==2 && g_static_s==3 && g_static_i==4 && g_static_l==5;
}
static int extern2() {
    extern char *g_extern_pc, g_extern_ac6[];
    extern int  *g_extern_pi, g_extern_ai4[];
    extern long *g_extern_pl, g_extern_al4[];

    return
        g_extern_pc[1]=='B' &&
        g_extern_pi[1]==10 &&
        g_extern_pl[1]==2 &&
        g_extern_ac6[2]=='C' &&
        g_extern_ai4[2]==20 &&
        g_extern_al4[2]==3;
}
static int extern3() {
    extern char  g_static_c;
    extern short g_static_s;
    extern int   g_static_i;
    extern long  g_static_l;
    return
        g_static_c==2 && g_static_s==3 && g_static_i==4 && g_static_l==5;
}
    extern int   g_extern4_i;
    extern int   g_extern4_i;
    extern int   g_extern4func(int);
    extern int   g_extern4func(int x);
static int extern4() {
    //関数内外で同じextern宣言があってもOK
    extern int   g_extern4_i;
    extern int   g_extern4_i;
    extern int   g_extern4func(int);
    extern int   g_extern4func(int x);
    return 1;
}
static int extern42() {
    //関数内外で同じextern宣言があってもOK
    extern int   g_extern4_i;
    extern int   g_extern4func(int);
    extern int   g_extern4func(int x);
    return 1;
}
static int static1() {
    static char  g_static_c=1;
    static short g_static_s=2;
    static int   g_static_i=3;
    static long  g_static_l=4;

    g_static_c --;
    g_static_s -= 1;
    g_static_i ++;
    g_static_l += 1;

    return
        ++g_static_c==1 &&
        ++g_static_s==2 &&
        --g_static_i==3 &&
        --g_static_l==4;
}
static int static2() {
    static char  g_static_c=11;
    static short g_static_s=22;
    static int   g_static_i=33;
    static long  g_static_l=44;

    int i, ret;
    for (i=10;i;i--) {
        static int cnt=0;       //初期化は1回だけ
        cnt++;
        ret = cnt;
    }
    return ret==10;
}
static int static3() {
    int a1,a2;
    {
        static int a=1;
        a1 = a;
    }
    {
        static int a=2;
        a2 = a;
    }
    return a1!=a2;
}
static int auto1() {
    auto char  auto_c=1;
    auto short auto_s=2;
    auto int   auto_i=3;
    auto long  auto_l=4;
    register char  register_c=1;
    register short register_s=2;
    register int   register_i=3;
    register long  register_l=4;

    return 1;
}
static const int const1() {
    const int i;
    int const j;
    const int *ip;
    int const *jp;
    const int const *const kp;
    const const int const ccic;
    return 1; 
}
static int ext() {
    TEST(extern1);
    TEST(extern2);
    TEST(extern3);
//    TEST(extern4);
    TEST(static1);
    TEST(static2);
    TEST(static3);
    TEST(auto1);
    TEST(const1);
    return 1;
}

static int declarate() {
    int (i)=1;
    char *(*c);
    int *(*(j));
    return 1;
}

static int cast(void) {
    char c1[]={1,2,3,4};
    char c2[]={0,0,0,0};
    *(int*)&c2 = 0x05040302;
    return
        *(int*)c1==0x04030201 &&
        c2[0]==2 && c2[3]==5;
}

    enum ABC ;
    enum ABC {AA,BB,C=100,D,E=C,} e1g_e=E;
    enum ABC ;
    static enum {P=-1,Q=-2,R=-3} e1g_ae;
    static int ABC;    //変数名とタグ名は名前空間が別
static int enum1g(void) {
    int x=0;
    e1g_ae = R;
    switch (e1g_e) {
    case AA:  x=AA+1; break;
    case E:  x=E+1; break;
    default: x=0;
    }
    {
        enum ABC e2;
        e2 = D;
    }
    return AA==0 && BB==1 && C==100 && D==C+1 && E==C && e1g_e==E && x==E+1 && e1g_ae==-3;
}
static int enum1(void) {
    enum ABC ;
    enum ABC {A,B,C=10,D,E=C,} e=E;
    static enum {P=-1,Q=-2,R=-3} ae; ae = R;
    int x=0;
    int ABC;    //変数名とタグ名は名前空間が別
    switch (e) {
    case A:  x=A+1; break;
    case E:  x=E+1; break;
    default: x=0;
    }
    {
        enum ABC e2;
        e2 = D;
    }
    return A==0 && B==1 && C==10 && D==C+1 && E==C && e==E && x==E+1 && ae==-3;
}
static int enum1e(void) {
    typedef enum ABC {A,B,C=10,D,E=C,} ABC;
    ABC e=E;
    typedef enum {P=-1,Q=-2,R=-3} PQ;
    static PQ ae; ae = R;
    int x=0;
    switch (e) {
    case A:  x=A+1; break;
    case E:  x=E+1; break;
    default: x=0;
    }
    {
        typedef enum ABC;
        ABC e2 = D;
    }
    return A==0 && B==1 && C==10 && D==C+1 && E==C && e==E && x==E+1 && ae==-3;
}
static int Enum(void) {
    TEST(enum1);
    TEST(enum1e);
    TEST(enum1g);
    return 1;
}

static int typedef1(void) {
    typedef int INT;
    INT i=5, j=i+1;
    INT a[]={10,20,30};
    INT *ip=&i;
    INT *pa[]={NULL, &i, &j};
    return i==5 && a[2]==30
        && *ip==5 && *pa[2]==6;
}
static int typedef1p(void) {
    typedef int INT;
    typedef int* INTP;
    INT i=5, j=i+1;
    INT a[]={10,20,30};
    INTP ip=&i;
    INTP pa[]={NULL, &i, &j};
    return i==5 && a[2]==30
        && *ip==5 && *pa[2]==6;
}
static int typedef1a(void) {
    typedef int INT;
    typedef int* INTP;
    typedef int INTA[];
    typedef int* INTPA[];
    INT i=5, j=i+1;
    INTA a={10,20,30};
    INTP ip=&i;
    INTPA pa={NULL, &i, &j};
    return i==5 && a[2]==30
        && *ip==5 && *pa[2]==6;
}
static int typedef3(void) {
    typedef char* STR;
    STR str="ABC";
    return strcmp(str, "ABC")==0;
}
static int Typedef(void) {
    TEST(typedef1);
    TEST(typedef1p);
    TEST(typedef1a);
    TEST(typedef3);
    return 1;
}

int main() {
    TEST(addsub);
    TEST(eq_rel);
    TEST(func);
    TEST(iterate);
    TEST(selection);
    TEST(incdec);
    TEST(pointer);
    TEST(array);
    TEST(string);
    TEST(init);
    TEST(align);
    TEST(size_of);
    TEST(type_of);
    TEST(scope);
    TEST(overflow);
    TEST(integer_def);
    TEST(ext);
    TEST(declarate);
    TEST(cast);
    TEST(Enum);
    TEST(Typedef);
    //printf("%s:%d func=%s\n",__FILE__, __LINE__, __func__);
    return 0;
}
