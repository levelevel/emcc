#ifdef _9cc
#define static
#define const
#define void
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#define TEST(func) if(!func()) {printf("Error at %s:%d:%s\n",__FILE__,__LINE__,#func);exit(1);}

int f42() {42; ;;;;;}
int addsub1(){
    int a; int b; int c; int sum;
    a = 1;
    b = 2 ;
    (c = + - -3);
    sum = a + b * -c;

    if (1) if (a==1) a = a + 1 ;

    return sum == 7 && a == 2;
}
int addsub() {
    return 
        42 == f42();    //これはCの仕様とは異なる
        14 == 10 + 2 * 3 - 4/2 &&
        3  == (((2+4)*1)/2) &&
        2  == +5%-(-3) &&
        addsub1() == 1 &&
        (1,2,3)==3 &&
        (8|7) == 15 &&
        (8^9) == 1 &&
        (15&3) == 3;
}

int eq_rel1() {
    return
        (5 == 5) == 1 &&
        (6 != 7) == 1 &&
        (1 <  2) == 1 &&
        (2 >  1) == 1 &&
        (3 <= 3) == 1 &&
        (4 >= 4) == 1;
}
int eq_rel2() {
    return
        (5 == 4) == 0 &&
        (6 != 6) == 0 &&
        (2 <  1) == 0 &&
        (1 >  2) == 0 &&
        (4 <= 3) == 0 &&
        (3 >= 4) == 0;
}
int eq_rel() {
    TEST(eq_rel1);
    TEST(eq_rel2);
    return 1;
}

int loop() {
    int a=3;
    while (a) a = a-1; 

    int b;
    for (b=0; b<5; b=b+1) b;
    int c;
    for (c=10; c>0; c--);
    //for (int i=10; i; i--) {};
    //for (;;) {}

    if(0);
    while(0);
    if(0) while(0) if(1) while(0);

    int i;
    int sum1 = 0, sum2 = 0;
    for (i=10;i>0;i--) {sum1=sum1+i;}
    while (i<=10) {sum2=sum2+i; i++;}

    int d=0; if(0) d=1; else d=2;
    int e=0; if(0) {e=1;} else if (1) {e=2;} else {e=3;}

    return a==0 && b==5 && c==0 && d==2 && e==2 && sum1==55 && sum2==55;
}

int inc() {
    int a=1, b, c;
    b = a++;
    c = ++b;
    return a+b+c==6;
}
int inc_char() {
    char a=1, b, c;
    b = a++;
    c = ++b;
    return a+b+c==6;
}
int dec() {
    int a=2, b, c;
    b = a--;
    c = --b;
    return a+b+c==3;
}
int dec_char() {
    char a=2, b, c;
    b = a--;
    c = --b;
    return a+b+c==3;
}
int incdec() {
    TEST(inc);
    TEST(inc_char);
    TEST(dec);
    TEST(dec_char);
    return 1;
}

int add(int a, int b){return a+b;}
int add3(int a, int b, int c){return a+b+c;}
int fact(int a) {
    if (a==0) return 0;
    else return a + fact(a-1);
}
int fib(int a) {
    if (a==0) return 0;
    else if (a==1) return 1;
    else return fib(a-1) + fib(a-2);
}
int func() {
    return
        fact(10) == 55 &&
        fib(10) == 55;
}

int pf1(int *p, int**pp) {
    return *p + **pp;
}
int pointer1() {
    int  a = 1;
    int *p = &a;
    return *p == 1 && pf1(&a, &p)==2;
}
    int  gp1_a = 1;
    int *gp1_p = &gp1_a;
int pointer1g() {
    return *gp1_p == 1 && pf1(&gp1_a, &gp1_p)==2;
}
int pointer2() {
    int a;
    int *p=&a;
    *p=12;
    return a==12;
}
    int gp2_a;
    int *gp2_p=&gp2_a;
int pointer2g() {
    *gp2_p=12;
    return gp2_a==12;
}
int**pointer3(int ***********a) {
    &(a);
    &(*(a));
    &**(**a);
    return 0;
}
int* pf4(int *a){return a;}
int pointer4() {
    int x;
    int *y=pf4(&x);
    return &x==y;
}
int pointer5() {
    int **p=0;
    p++;
    ++p;
    return p-1 == sizeof(int*);
}
int pointer6() {
    int *p=0;
    int a=2;
    p = p+a;
    p = p+1;
    p--;
    --p;
    return 1+p == sizeof(int)*2;
}
int pointer() {
    TEST(pointer1);
    TEST(pointer1g);
    TEST(pointer2);
    TEST(pointer2g);
    TEST(pointer4);
    TEST(pointer5);
    TEST(pointer6);
    return 1;
}

int array1() {
    int a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        a==&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
int array1c() {
    char a[4];
    *a     = 2;
    a[1]   = 4;
    *(a+2) = 8;
    a[3]   = 16;
    return
        a==&a && a[0]==2 && *(a+1)==4 && a[2]==8 && (1,2)[1+a]==16;
}
int af2(int *a){return a[0]+a[1]+a[2];}
int array2() {
    int a[4]={1,2,3,4};
    return af2(a)==6;
}
char af2c(char *a){return a[0]+a[1]+a[2];}
int array2c() {
    char a[4]={1,2,3,4};
    return af2c(a)==6;
}
int array3(int argc, char*argv[]) { //コンパイルのみ
    return 1;
}
int array() {
    TEST(array1);
    TEST(array1c);
    TEST(array2);
    TEST(array2c);
    return 1;
}

    char sg1_buf[20];
    char*sg1_p[4];
    char sg1_str1[5] = "ABC";
    char sg1_str2[ ] = "ABC";
    char sg1_str3[4] = "ABCDE"; //長すぎる
int string1() {
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
int string1g() {
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
int string() {
    TEST(string1);
    TEST(string1g);
    return 1;
}

int init1() {
    int a[] = {1,2,1+2};
    int b = {5,};
    char s1[] = "ABC";
    char s2[] = {'A', 66, 'A'+2, 0};
    int i=1;
    char ac[]={i,i*2,10};
    int  ai[]={i,i*2,10};

    return
        a[0]+a[1]+a[2]==6 && sizeof(a)==sizeof(int)*3 &&
        b==5 && 
        strcmp(s1,s2)==0 &&
        ac[0]+ac[1]+ac[2]==13 &&
        ai[0]+ai[1]+ai[2]==13;
}
    int i1g_a[] = {1,2,1+2};
    int i1g_b = {5,};
    char i1g_s1[] = "ABC";
    char i1g_s2[] = {'A', 66, 'A'+2, 0};
int init1g() {
    return
        i1g_a[0]+i1g_a[1]+i1g_a[2]==6 && sizeof(i1g_a)==sizeof(int)*3 &&
        i1g_b==5 && 
        strcmp(i1g_s1,i1g_s2)==0;
}
int  i2g_x, *i2g_p = 2 + &i2g_x - 1;
char i2g_c, *i2g_ac[3] = {0,&i2g_c+1,3,4};    //初期値多い
int  i2g_i, *i2g_ai[5] = {0,&i2g_i+1,3,4};    //初期値足りない分は0
int init2g() {
    return 
        &i2g_x+1 == i2g_p && i2g_x==0 &&
        i2g_ac[1]==&i2g_c+1 && i2g_ac[2]==3 &&
        i2g_ai[1]==&i2g_i+1 && i2g_ai[2]==3 && i2g_ai[4]==0;
}
int init() {
    TEST(init1);
    TEST(init1g);
    TEST(init2g);
    return 1;
}

int align1() {
    char c1; int i; char c2; char*p;
    return &i%4==0 && &p%8==0;
}
    char ag1_c1; int ag1_i; char ag1_c2; char*ag1_p;
int align1g() {
    return &ag1_i%4==0 && &ag1_p%8==0;
}
int align() {
    TEST(align1);
    TEST(align1g);
    return 1;
}

int size_of1() {
    int a; int *p; 
    return
        sizeof(a)==4 && sizeof(p)==8 && sizeof(1)==4 && sizeof(&p)==8 && 
        sizeof(char)==1 && sizeof(int)==4 && sizeof(int*)==8 &&
        sizeof(char[5])==5 && sizeof(int[5])==4*5 && sizeof(int*[3])==8*3 &&
        sizeof(1&&1==1>1)==4 && sizeof(a=1)==4;
}
int size_of2() {
    int a[2*5];
    return
        sizeof(a)==40 && sizeof(int*[2*2])==32;
}
int size_of3() {
    return
        _Alignof(char)==1 && _Alignof(int)==4 && _Alignof(int*)==8 &&
        _Alignof(char[5])==1 && _Alignof(int[5])==4 && _Alignof(int*[3])==8;
}
int size_of() {
    TEST(size_of1);
    TEST(size_of2);
    TEST(size_of3);
    return 1;
}

    int sc_x=0, sc_y=0;
int scope() {
    int sc_x; 
    sc_x=1;
    sc_y=2;
    return sc_x+sc_y==3;
}

int overflow() {
    char c=255;
    char i=0xffff;
    return
        c == -1 && c<0 &&
        i == -1 && i<0; 
}

int main() {
    TEST(addsub);
    TEST(eq_rel);
    TEST(func);
    TEST(loop);
    TEST(incdec);
    TEST(pointer);
    TEST(array);
    TEST(string);
    TEST(init);
    TEST(align);
    TEST(size_of);
    TEST(scope);
    TEST(overflow);
    return 0;
}
