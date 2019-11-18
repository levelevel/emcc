#include "../9cc.h"

struct {
    enum {ER, WR, OK, CM} expect;
    char *code;
    char *note;
} test[] = {
    {ER, "42"},
    {ER, "10 + / 2;"},
    {ER, "main(){}"},
    {ER, "int main(a){}"},
    {ER, "int main(1){}"},
    {ER, "int main(int a+1){}"},
    {ER, "int main(int a,){}"},
    {ER, "a;"},
    {ER, "a[1];"},
    {ER, "a++;"},
    {ER, "int a; int a;",                       "ローカル変数の重複定義"},
    {ER, "int a=1; int a=1;",                   "ローカル変数の重複定義"},
    {ER, "static int a=1, a=1;",                "静的ローカル変数の重複定義"},
    {ER, "int a=1, a=1; void main(){}",         "グローバル変数の重複定義"},
    {ER, "static int a; int a;",                "ローカル変数のstatic/auto"},
    {ER, "static int a; int a;void main(){}",   "グローバル変数のstatic/global"},
    {ER, "int &a;"},
    {ER, "int +a;"},
    {ER, "return 1++;"},
    {ER, "return --1;"},
    {ER, "int a; *a;"},
    {ER, "int *a; **a;"},
    {ER, "*1;"},
    {ER, "*1=0;"},
    {ER, "&1; return 1;"},
    {ER, "int a; *a;"},
    {ER, "int a; *a=0;"},
    {ER, "int a; & &a; return 1;"},
    {WR, "int *a; a=1;"},
    {WR, "int a; int*b; a=b;"},
    {WR, "int a; int*b; b=a;"},
    {WR, "int a; int b; a=&b;"},
    {WR, "int*a; int**b; a=b;"},
    {WR, "int *f(){int a; return &a;} int main(){int a; a=f();}"},
    {ER, "int *p; p+p;"},
    {ER, "int *p; char*q; p-q;"},
    {ER, "int *p; 1-p;"},
    {ER, "int *p; return sizeof(**p);"},
    {ER, "int a[4]; a=1;"},
    {ER, "char *argv[];"},
    {ER, "char *argv[0];"},
    {ER, "int a; char *argv[a];"},
    {ER, "char  char a;"},
    {ER, "short short a;"},
    {ER, "int   int a;"},
    {ER, "long long long a;"},
    {ER, "void  void *p;"},
    {ER, "signed void *p;"},
    {ER, "unsigned void *p;"},
    {ER, "void a;"},
    {ER, "signed unsigned a;"},
    {ER, "signed _Bool b;"},
    {ER, "unsigned _Bool b;"},
    {ER, "static static int a;"},
    {ER, "extern extern int a;"},
    {ER, "extern static int a;"},
    {ER, "extern int x; extern int*x;"},
    {ER, "int main(){extern int x; extern int*x;}"},
    {ER, "static extern func(){} int main(){}"},
    {ER, "sizeof(static int);"},
    {ER, "sizeof(extern int);"},
    {ER, "sizeof(int[])"},
    {ER, "int main(...){}"},
    {ER, "int main(int argc, ..., char *argv[]){}"},
    {ER, "int main(int argc, void){}"},
    {ER, "int func(int , void); int main(){}"},
    {ER, "int func(int a, char *a); int main(){}"},
    {WR, "int a = func();"},
    {ER, "void func(){} void func(){} int main(){}"},
    {ER, "int func;     void func(){} int main(){}"},
    {ER, "void func(){} int func;     int main(){}"},
    {ER, "int func(); *func();"},
    {ER, "int func(); int (*fp)()=func; *fp();"},
    {ER, "int func(); func()();"},
    {ER, "int func(); func()[];"},
    {ER, "int a; a();"},
    {ER, "int a; int main(){a();}"},
    {ER, "extern int func(); extern int*func();"},
    {ER, "static void func();",                 "関数内でstaticな関数の宣言"},
    {ER, "break; return 1;"},
    {ER, "continue; return 1;"},
    //LVALUE
    {ER, "int a,b; (1?a:b)=0;"},
    {ER, "int a; a||1=0;"},
    {ER, "int a; a&&1=0;"},
    {ER, "int a; a^1=0;"},
    {ER, "int a; a|1=0;"},
    {ER, "int a; a&1=0;"},
    {ER, "int a; a==1=0;"},
    {ER, "int a; a>1=0;"},
    {ER, "int a; a+1+=0;"},
    {ER, "int a; a*1-=0;"},
    {ER, "int *a; (char*)a=0;"},
    {ER, "int a; a&1=0;"},
    {ER, "int a; a|1=0;"},
    {ER, "int a; a^1=0;"},
    {ER, "int a; ~a=0;"},
    {ER, "int a; a>>1=0;"},
    {ER, "int a; a<<1=0;"},
    {ER, "int*p; p&1;"},
    {ER, "int*p; p|1;"},
    {ER, "int*p; p^1;"},
    {ER, "int*p; ~p;"},
    {ER, "int*p; p>>1;"},
    {ER, "int*p; 1<<p;"},
    {WR, "void main(){return 1;}"},
    {WR, "int main(){return;}"},
    {WR, "int main(){char*p=0; return p;}"},
    {ER, "goto; return 1;"},
    {ER, "goto L1; return 1;"},
    {ER, "L1:; L1:; return 1;"},
    {ER, "case 1: ;"},
    {ER, "default 1: ;"},
    {ER, "switch(1){case 1:; case 1:;}"},
    {ER, "switch(1){default:; default:;}"},
    {ER, "int main(void); int main(int x){return 1:}"},
    {ER, "int main(void); int main(int){return 1:}"},
    {ER, "int main(void); int main(int x);"},
    {ER, "void main(void){} void main(int);"},
    {ER, "int main(int); int main(int, ...);"},
    {ER, "int main(int); int main(){return 1;};"},
    {ER, "int func(void); int main(){int func(int);}"},
    {ER, "int main(){int func(void); int func(int);}"},
    {ER, "typedef int INT=1;"},
    {ER, "typedef int INT; unsigned INT i;"},
    {ER, "int func(int a){return 1;}; int main(){return func();}",          "引数1個に対して0個でコール"},
    {ER, "int func(int a, int b){return 1;}; int main(){return func(1);}",  "引数2個に対して1個でコール"},
    {ER, "int func(int a){return 1;}; int main(){return func(1,2);}",       "引数1個に対して2個でコール"},
    {ER, "int func(void){return 1;}; int main(){return func(1);}",          "引数0個(void)に対して1個でコール"},
    {ER, "int func(int a, int b, ...){return 1;}; void main(){func(1);}",   "引数2+...個に対して1個でコール"},
    {WR, "int func(int*a){return 1;}; int main(){return func(1);}",         "int*に対してintでコール"},
    {WR, "int func(int a){return 1;}; int main(){int a; return func(&a);}", "intに対してint*でコール"},
    {ER, "int a, b=a; void main(){}",          "グローバル変数をグローバル変数で初期化"},
    {ER, "int a; void main(){static int b=a;", "静的ローカル変数をローカル変数で初期化"},
    {ER, "static int a, b=a;",                 "静的ローカル変数を静的ローカル変数で初期化"},
    {ER, "int a; static int b=a;",             "静的ローカル変数を自動変数で初期化"},
    {ER, "int a[4]; static int *b=a;",         "静的ローカル変数を自動変数で初期化"},
    //数値
    {ER, "return 0xx5;",                       "不正な16進"},
    {ER, "return 08;",                         "不正な8進"},
    {ER, "return 5uu;",                        "uの重複"},
    {ER, "return 5ulu;",                       "uの重複"},
    {ER, "return 5llul;",                      "lの重複"},
    {ER, "return 5x;",                         "ゴミ"},
    {ER, "return 5LUx;",                       "ゴミ"},
    //文字
    {ER, "return 'ab';",                       "複数文字"},
    {WR, "return '\\N';",                      "未定義のエスケープシーケンス"},
    {ER, "return '\\0123';",                   "4文字以上のoctal"},
    {ER, "return '\\08';",                     "octal規定外文字"},
    {ER, "return '\\xG';",                     "hexadecimal規定外文字"},
    //多次元配int func(int a){return 1;}; int main(){return func();}列
    {ER, "int a[][3]={{1,2,3},{11,12,13}; return a[1][2]}"},
    {ER, "int x; int x[4]; int main(){}"},
    {ER, "\"ABC\"=1;"},
    {ER, "char p[3]=1;"},
    {ER, "int a[]=\"ABC\";"},
    {ER, "char *p[]=\"ABC\";"},
    {ER, "int a; a[1];"},

    {CM, "===== LOCAL配列の初期化 ====="},
    {WR, "char a[3]=\"abc\"; return 0;"},
    {WR, "int a[2]={1,2,3}; return 0;"},
    {WR, "int a[2][2]={{1,2,3},{4,5,6}}; return 0;"},
    {WR, "int a[2][2]={{1,2},{3,4},{5,6}}; return 0;"},
    {WR, "char a[2][2]={\"ab\",\"cd\"}; return 0;"},
    {WR, "int a[3]={1,{2,99},3}; return 0;"},
    {WR, "int x=1, a[2]={x,2,3}; return 0;",     "LOCAL配列の初期化:非定数式"},
    {WR, "int x=1, a[2][2]={{x,2,3},{4,5,6}}; return 0;"},
    {WR, "int x=1, a[2][2]={{x,2},{3,4},{5,6}}; return 0;"},

    {CM, "===== LOCAL STATIC配列の初期化 ====="},
    {WR, "static char a[3]=\"abc\"; return 0;"},
    {WR, "static int a[2]={1,2,3}; return 0;"},
    {WR, "static int a[2][2]={{1,2,3},{4,5,6}}; return 0;"},
    {WR, "static int a[2][2]={{1,2},{3,4},{5,6}}; return 0;"},
    {WR, "static char a[2][2]={\"ab\",\"cd\"}; return 0;"},
    {WR, "static int a[3]={1,{2,99},3}; return 0;"},

    {CM, "===== GLOBAL配列の初期化 ====="},
    {WR, "char a[3]=\"abc\"; void main(){}"},
    {WR, "int a[2]={1,2,3}; void main(){}"},
    {WR, "int a[2][2]={{1,2,3},{4,5,6}}; void main(){}"},
    {WR, "int a[2][2]={{1,2},{3,4},{5,6}}; void main(){}"},
    {WR, "char a[2][2]={\"ab\",\"cd\"}; void main(){}"},
    {WR, "int a[3]={1,{2,99},3}; void main(){}"},

    {ER, "_Static_assert(0,\"aaa\");"},
    {ER, "_Static_assert(0,\"aaa\"); void main(){}"},
    {ER, "int a; _Static_assert(a,\"aaa\");"},
    {ER, "int a[2]; _Static_assert(sizeof(a)==1,\"aaa\");"},

    {CM, "===== 列挙型 ====="},
    {ER, "struct E; enum E;"},
    {ER, "enum E e;"},
    {ER, "return sizeof(enum E);"},
    {ER, "enum E{};"},
    {ER, "enum E{A}; enum E{A};"},
    {ER, "enum E{A}; enum E{B};"},
    {ER, "enum E; enum E{A,B}; enum E{C,D};"},
    {ER, "int e; enum ABC{A,B,C} e;"},
    {ER, "enum ABC{A,B,C} e; int e;"},
    {ER, "enum ABC{A,B,A} e;"},
    {ER, "enum ABC{A,B,C} e; enum ABC{X,Y,Z} x;"},
    {ER, "enum ABC{A,B,C} e; enum XYZ{A,Y,Z} x;"},
    {ER, "enum ABC{A,B,C}; enum ABC{P,Q,R} e;"},
    {ER, "enum ABC{A,B,C}; unsigned enum ABC abc;"},

    {CM, "===== 構造体 ====="},
    {ER, "enum S; struct S;"},
    {ER, "struct S e;"},
    {ER, "struct S{}e;"},
    {ER, "struct S{int a;}e; struct S{int a;}e2;"},
    {ER, "struct S{int a; int a;}s;"},
    {ER, "struct S{int a; int b;}; unsigned struct S s;"},
    {ER, "int a; return a.b;"},
    {ER, "int a; return a->b;"},
    {ER, "struct S{int a;}s; return s.x"},
    {ER, "struct S{int a;}s; return s->x"},
    {ER, "typedef struct ST st_t;{struct ST{int a,b;}; st_t st;}", "スコープの違い"},
    {ER, "struct{int a;}st; st=1;"},
    {ER, "struct{int a;}st; int b=st;"},
    {ER, "struct{int a;}st; int b; b=st;"},
    {ER, "struct{int a;}st1; struct{int b;}st2=st1;"},
    {ER, "struct{int a;}st1; struct{int b;}st2; st1=st2;"},
    {ER, "struct{int a;}st; return st;"},
    {WR, "struct{int a;}st; return &st;"},
    {ER, "struct{int a;}st; if(st);"},
    {ER, "struct{int a;}st; switch(st){};"},
    {ER, "struct{int a;}st; while(st)a;"},
    {ER, "struct{int a;}st; do;while(st);"},
    {ER, "struct{int a;}st; for(;st;;)break;"},
    {ER, "struct{int a;}st; st==1; return 0;"},
    {ER, "struct{int a;}st; st<1; return 0;"},
    {ER, "struct{int a;}st; st>>1; return 0;"},
    {ER, "struct{int a;}st; 1*st; return 0;"},
    {ER, "struct{int a;}st; st/1; return 0;"},
    {ER, "struct{int a;}st; +st; return 0;"},
    {ER, "struct{int a;}st; -st; return 0;"},
    {ER, "struct{int a;}st; !st; return 0;"},
    {ER, "struct{int a;}st; ~st; return 0;"},
    {ER, "struct{int a;}st; ++st; return 0;"},
    {ER, "struct{int a;}st; --st; return 0;"},
    {ER, "struct{int a;}st; st++; return 0;"},
    {ER, "struct{int a;}st; st--; return 0;"},
    {ER, "typedef struct{int a;}ST; ST st; void func(int); func(st);"},
    {WR, "typedef struct{int a;}ST; ST st; void func(int); func(&st);"},
    {ER, "typedef struct{int a;}ST; ST st; void func(ST); func(1);"},
    {ER, "typedef struct{int a;}ST; ST st; void func(ST); func(&st);"},

    {CM, "===== 無意味な宣言 ====="},
    {WR, "static struct ST{int a;}; return 0;"},
    {WR, "const  struct ST{int a;}; return 0;"},
    {WR, "static union UN{int a;}; return 0;"},
    {WR, "const  union UN{int a;}; return 0;"},
    {WR, "static struct ST{int a;}; void main(){}"},
    {WR, "const  struct ST{int a;}; void main(){}"},
    {WR, "static union UN{int a;}; void main(){}"},
    {WR, "const  union UN{int a;}; void main(){}"},

    {CM, "===== 共用体 ====="},
    {ER, "struct U; union U;"},
    {ER, "union U u;"},
    {ER, "union U{};"},
    {ER, "union U{int a;}; union U{int a;};"},
    {ER, "union U{int a; int a;};"},
    {ER, "union U{int a; int b;}u; int u;"},
    {ER, "union U{int a; int b;}; signed union U u;"},
    {ER, "union U{int a; int b;}u; return u.x;"},
    {ER, "union U{int a; int b;}u; return u->x;"},
    {ER, "typedef union UN un_t;{union UN{int a,b;}; un_t un;}", "スコープの違い"},
    {ER, "union{int a;}un1; union{int b;}un2; un1=un2;"},
    {ER, "union{int a;}un; return un;"},
    {WR, "union{int a;}un; return &un;"},
    {ER, "typedef union{int a;}UN; UN un; void func(int); func(un);"},
    {WR, "typedef union{int a;}UN; UN un; void func(int); func(&un);"},
    {ER, "typedef union{int a;}UN; UN un; void func(UN); func(1);"},
    {ER, "typedef union{int a;}UN; UN un; void func(UN); func(&un);"},

    {CM, "===== 構造体・共用体の初期化 ====="},
    {WR, "struct{int x,y;}st={1,2,3}; return 0;",                           "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int a,b; struct{int x,y;}s;}st={0,0,{1,2,3}}; return 0;",  "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int a,b; struct{int x,y;} ;}st={0,0,{1,2,3}}; return 0;",  "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "union {int x,y;}un={1,2}; return 0;",                             "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int a,b; union{int x,y;}u;}un={0,0,{1,2}}; return 0;",     "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int a,b; union{int x,y;} ;}un={0,0,{1,2}}; return 0;",     "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int x,y;}st={1,{2,3}}; return 0;",                         "スカラーがリストで初期化されています"},
    {WR, "union {int x,y;}st={{2,3}}; return 0;",                           "スカラーがリストで初期化されています"},
    {WR, "static struct{int x,y;}st={1,2,3}; return 0;",                           "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int a,b; struct{int x,y;}s;}st={0,0,{1,2,3}}; return 0;",  "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int a,b; struct{int x,y;} ;}st={0,0,{1,2,3}}; return 0;",  "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static union {int x,y;}un={1,2}; return 0;",                             "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int a,b; union{int x,y;}u;}un={0,0,{1,2}}; return 0;",     "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int a,b; union{int x,y;} ;}un={0,0,{1,2}}; return 0;",     "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int x,y;}st={1,{2,3}}; return 0;",                         "スカラーがリストで初期化されています"},
    {WR, "static union {int x,y;}st={{2,3}}; return 0;",                           "スカラーがリストで初期化されています"},

    {CM, "===== 無名構造体・共用体 ====="},
    {WR, "struct {int sa; long sb;};",                          "ネストしていない無名構造体:"},
    {WR, "union  {int ua; long ub;};",                          "ネストしていない無名共用体:"},
    {ER, "struct{int a; union U{int ua; long ub;};};",          "構造体の中の無名共用体:タグ付きは変数名必要"},
    {ER, "struct{int a; union  {int  a; long ub;};};",          "構造体の中の無名共用体:変数名重複:前"},
    {ER, "struct{int a; union  {int ua; long ub;}; int ua};",   "構造体の中の無名共用体:変数名重複:後"},
    {ER, "union{int a; struct S{int sa; long sb;};};",          "共用体の中の無名構造体:タグ付きは変数名必要"},
    {ER, "union{int a; struct  {int  a; long sb;};};",          "共用体の中の無名構造体:変数名重複:前"},
    {ER, "union{int a; struct  {int sa; long sb;}; int sa};",   "共用体の中の無名構造体:変数名重複:後"},
    {WR, "static struct {int sa; long sb;};",                          "ネストしていない無名構造体:"},
    {WR, "static union  {int ua; long ub;};",                          "ネストしていない無名共用体:"},
    {ER, "static struct{int a; union U{int ua; long ub;};};",          "構造体の中の無名共用体:タグ付きは変数名必要"},
    {ER, "static struct{int a; union  {int  a; long ub;};};",          "構造体の中の無名共用体:変数名重複:前"},
    {ER, "static struct{int a; union  {int ua; long ub;}; int ua};",   "構造体の中の無名共用体:変数名重複:後"},
    {ER, "static union{int a; struct S{int sa; long sb;};};",          "共用体の中の無名構造体:タグ付きは変数名必要"},
    {ER, "static union{int a; struct  {int  a; long sb;};};",          "共用体の中の無名構造体:変数名重複:前"},
    {ER, "static union{int a; struct  {int sa; long sb;}; int sa};",   "共用体の中の無名構造体:変数名重複:後"},
    
    {CM, "===== 構造体・共用体の配列の初期化 ====="},
    {WR, "struct{int a,b;}s[2]={1,2,3,4,5}; return 0;",         "初期化リストが配列サイズを超えています"},
    {WR, "struct{int a,b;}s[2]={{1,2,9},{3,4,9}}; return 0;",   "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "struct{int a,b;}s[2]={1,{2},3,4}; return 0;",         "スカラーがリストで初期化されています"},
    {WR, "static struct{int a,b;}s[2]={1,2,3,4,5}; return 0;",         "初期化リストが配列サイズを超えています"},
    {WR, "static struct{int a,b;}s[2]={{1,2,9},{3,4,9}}; return 0;",   "構造体・共用体の初期化リストが要素数を超えています"},
    {WR, "static struct{int a,b;}s[2]={1,{2},3,4}; return 0;",         "スカラーがリストで初期化されています"},

    {CM, "===== Const ====="},
    {ER, "const int ci; ci=1; return ci;"},
    {ER, "static char str[]=\"abc\"; const char*p1=str; *p1='A';"},
    {ER, "static char str[]=\"abc\"; char const*p2=str; *p2='A';"},
    {ER, "static char str[]=\"abc\"; const char const*p12=str; *p12='A';"},
    {ER, "static char str[]=\"abc\"; char*const p3=str; p3=0;"},
    {ER, "static char str[]=\"abc\"; const char*const p13=str; p13=0;"},
    {ER, "static char str[]=\"abc\"; const char*p1=str; (*p1)+=1;"},
    {ER, "static char str[]=\"abc\"; const char*p1=str; (*p1)++;"},
    {WR, "char*func(void) {static const char str[]=\"abc\"; return str;} void main(){}"},
    {ER, "void func3(char*str){}\
          void func3(const char*str); void main(){}"},
    {ER, "char*func3(void){return 0;}\
          const char*func3(void); void main(){}"},
    {WR, "void func(char*p); const char*p; func(p); return 0;",     "戻り値const char*のconst情報は失われます"},
    {OK, "void func(char*p); char* const p; func(p); return 0;"},
    {OK, "void func(const char*p); char*p; func(p); return 0;"},

    {CM, "===== Assign ====="},
    {ER, "int a; char*p; a+=p;"},
    {ER, "int a; char*p; a-=p;"},
    {ER, "int a; struct{int x;}s; a*=s;"},
    {ER, "int a; union {int x;}u; a/=u;"},
    {ER, "int a; struct{int x;}s; a%=s;"},

    {OK, "#include test_src/9cc.c"},
    {ER, NULL}
};

jmp_buf jmpbuf;

static int test_cnt = 0;
static int ok_cnt   = 0;
static int ng_cnt   = 0;

static void test_error1(int index) {
    char fname[50];
    FILE *fp = stderr;

//  sprintf(fname, "selftest%03d", index);
    sprintf(fname, "selftest");
    filename = fname;

    if (test[index].expect==CM) {
        fprintf(fp, "# %s\n", test[index].code);
        return;
    }

    user_input = test[index].code;
    if (strncmp(user_input, "#include", 8)==0) {
        char *p = user_input+8;
        while (isspace(*p)) p++;
        user_input = read_file(p);
    } else if (strstr(user_input, "main")==NULL) {
        char *buf = malloc(strlen(user_input) + 50);
        sprintf(buf, "int main(){%s}", user_input);
        user_input = buf;
    }

    error_ctrl   = ERC_LONGJMP; //エラー発生時にlongjmp
    warning_ctrl = (test[index].expect==WR ? ERC_LONGJMP : ERC_CONTINUE);
    if (setjmp(jmpbuf)==0) {
        fprintf(fp, "# %s ----------------\n", filename);
        if (test[index].note) fprintf(fp, "# [%s]\n", test[index].note);
        fprintf(fp, "%s\n", user_input);
        compile();
    }

    test_cnt++;
    switch (test[index].expect) {
    case ER:
        if (error_cnt==0) {
            fprintf(fp, "  ---> NG! (expected: Error)\n");
            ng_cnt++;
        } else {
            ok_cnt++;
        }
        break;
    case WR:
        if (warning_cnt==0) {
            fprintf(fp, "  ---> NG! (expected: Warning)\n");
            ng_cnt++;
        } else {
            ok_cnt++;
        }
        break;
    case OK:
        if (error_cnt || warning_cnt) {
            fprintf(fp, "  ---> NG! (expected: No message)\n");
            ng_cnt++;
        } else {
            ok_cnt++;
        }
        break;
    default:
        break;
    }
}

void test_error(void) {
    for (int i=0; test[i].code; i++) {
        test_error1(i);
    }
    fprintf(stderr, "9cc self test\n");
    fprintf(stderr, "Total : %d\n", test_cnt);
    fprintf(stderr, "   Ok : %d\n", ok_cnt);
    fprintf(stderr, "   NG : %d\n", ng_cnt);
}
