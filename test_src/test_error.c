#include "../9cc.h"

struct {
    enum {ER, WR} expect;
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
    {ER, "ststic ststic int a;"},
    {ER, "extern extern int a;"},
    {ER, "extern ststic int a;"},
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
    {ER, "int*p; p<<1;"},
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
    {ER, "int e; enum ABC{A,B,C} e;"},
    {ER, "enum ABC{A,B,C} e; int e;"},
    {ER, "enum ABC{A,B,A} e;"},
    {ER, "enum ABC{A,B,C} e; enum ABC{X,Y,Z} x;"},
    {ER, "enum ABC{A,B,C} e; enum XYZ{A,Y,Z} x;"},
    {ER, "enum ABC{A,B,C}; enum ABC{P,Q,R} e;"},
    {ER, "enum ABC{A,B,C}; unsigned enum ABC abc;"},
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
    {ER, "int func(int *a){return 1;}; int main(){return func(1);}",        "int*に対してintでコール"},
    {ER, "int a, b=a; void main(){}",          "グローバル変数をグローバル変数で初期化"},
    {ER, "int a; void main(){static int b=a;", "静的ローカル変数をローカル変数で初期化"},
    {ER, "static int a, b=a;",                 "静的ローカル変数をローカル変数で初期化"},
    {ER, "int a; static int b=a;",             "静的ローカル変数を自動変数で初期化"},
    //多次元配int func(int a){return 1;}; int main(){return func();}列
    {ER, "int a[][3]={{1,2,3},{11,12,13}; return a[1][2]}"},
    {ER, "int x; int x[4]; int main(){}"},
    {ER, "\"ABC\"=1;"},
    {ER, "char p[3]=1;"},
    {ER, "int a[]=\"ABC\";"},
    {ER, "char *p[]=\"ABC\";"},
    {ER, "int a; a[1];"},
    {ER, NULL}
};

jmp_buf jmpbuf;

static int test_cnt = 0;
static int ok_cnt   = 0;
static int ng_cnt   = 0;

static void test_error1(int index) {
    char fname[50];
    FILE *fp = stderr;
    sprintf(fname, "selftest%03d", index);
    filename = fname;
    user_input = test[index].code;
    if (strstr(user_input, "main")==NULL) {
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