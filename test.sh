#!/bin/bash
set -u

EXE=tmp
AFLAGS="-g -no-pie"
ER=Error
WR=Warning

rm -f $EXE.log

GDB=""
test_er_only=0
cnt=0

try() {
  expected="$1"
  input="$2"
  let cnt++
  rm -f $EXE

  echo "=== test[$cnt] ================================" >> $EXE.log
  if [ $expected == $ER -o $expected == $WR ]; then
    ./9cc "$input" 2>&1 > $EXE.s | tee -a $EXE.log | grep "9cc:$expected" > $EXE.err
    if [ $? -eq 0 ]; then
      actual=$expected
    else
      actual="Normal End"
    fi
    if [ $test_er_only -ne 0 ]; then cat $EXE.err; fi
    rm -f $EXE.err
  else
    if [ $test_er_only -ne 0 ]; then return; fi
    ./9cc "$input" > $EXE.s
    gcc $AFLAGS -o $EXE $EXE.s func.o
    ./$EXE
    actual="$?"
  fi

  if [ "$actual" == "$expected" ]; then
    echo "# $input => $actual" | tee -a $EXE.log
  else
    echo "#! $input => $expected expected, but got $actual" | tee -a $EXE.log
    exit 1
  fi 

  cat $EXE.s >> $EXE.log
  echo >> $EXE.log
}

try1() {
  rm -f $EXE
  make -s
  ./9cc "$*" > $EXE.s
  if [ $? != 0 ]; then exit 1; fi
  cat -n $EXE.s
  gcc $AFLAGS -o $EXE $EXE.s func.o
  if [ $? -eq 0 ]; then
    $GDB ./$EXE
    echo $?
  fi
  exit $?
}

while [ $# -gt 0 ]; do
  case $1 in
  -d*) GDB=gdb;;
  -e*) test_er_only=1;;
  *) try1 "$1";;
  esac
  shift
done

try 42 "42;"
try $ER "42"
try 14 "10 + 2 * 3 - 4/2;"
try $ER "10 + / 2;"
try 3 "(((2+4)*1)/2);"
try 2 "+5%-(-3);"
try 6 "(1<2) + (2>1) + (3<=3) + (4>=4) + (5==5) + (6!=6+1);"
try 0 "(2<1) + (1>2) + (4<=3) + (3>=4) + (5==5-1) + (6!=6);"
try 7 "int a; int b; int c; int sum; a=1; b = 2 ;(c= -3); sum = a+b* -c; return sum;"
try 3 "int a; a=2; if(1)if(a==2)a=a+1;a;"
try 0 "int a; a=3; while(a)a=a-1;a;"
try 5 "int a; for(a=0;a<5;a=a+1)a;return a;5;"
try 0 "if(0);while(0);"
try 0 "if(0)while(0)if(1)while(0);"
try 3 "int a; a=3;for(;;)return a;"
try 5 "5; ;;;;;;;;"
try 5 "int a; a = + - (+ - +5);"
try 7 "int a; int b; a=1;b=2; if(a+b>0){a=a+1;b=5;a+b;}"
try 6 "int a; int b; int c; a=1;b=a++;c=++b;a+b+c;"
try 3 "int a; int b; int c; a=2;b=a--;c=--b;a+b+c;"
try 55 "int i; int sum; sum=0; for(i=10;i>0;i--) {sum=sum+i;} return sum;"
try 55 "int i; int sum; sum=0; i=0; while(i<=10) {sum=sum+i; i++;} if(!!1)return sum;"
try 2 "int a; a=0; if(0) a=1; else a=2; a;"
try 2 "int a; a=0; if(0) {a=1;} else if (1) {a=2;} else {a=3;} a;"
try 1 "int true; int false; true=1&&1&&1;false=0&&1;true==1&&false==0;"
try 1 "int true; int false; int a; int b; a=1;b=0;true=a||b;false=b||0;true==1&&false==0;"
try 0 "int ret; ret=1;ret=foo(); bar(); ret;"
try 0 "int a; int ret; a=2;ret=1;foo(); ret=bar(); ret;"
try 3 "return (1,2,3);"
try 1 "int a; int b; int c; a=1,b=2,c=3; a==1 && b==2 && c==3;"
try 1 "1!=2 && 2==2 && 1>0;"
try 0 "1!=1 || 2==2+1 || 1>=2;"
try 2 "int ret1; ret1=func1(1);"
try 8 "int x; int r1; x=1; r1=func3(x*2,(2+1),3);"

try 1 "int main(){1;}"
try 1 "int func(){return 1;} int main(){return func();}"
try 15 "int main(){return add(1,2)+add3(3,4,5);} 
        int add(int a, int b){return a+b;} 
        int add3(int a, int b, int c){return a+b+c;}"
try 106 "int main(){int a; int b; a=1;b=2; return func(a,b+1);} 
        int func(int a, int b){int c; int sum; c=100;a=b; sum=a+b+c; return sum;}"
try 55 "int main() {
          return fact(10);
        } 
        int fact(int a) {
          if (a==0) return 0;
          else return a + fact(a-1);
        }"
try 55 "int main() {
          return fib(10);
        } 
        int fib(int a) {
          if (a==0) return 0;
          else if (a==1) return 1;
          else return fib(a-1) + fib(a-2);
        }"
try $ER "main(){}"
try $ER "int main(a){}"
try $ER "int main(1){}"
try $ER "int main(int a+1){}"
try $ER "int main(int a,){}"
try $ER "a;"
try $ER "int a; int a;"
try $ER "int (a);"
try $ER "int &a;"
try $ER "int +a;"
try $ER "1++;"
try $ER "--1;"

try 1 "int main() {int a; int *b; a=1; b=&a; return *b;}"
try 20 "int main() {
          int a; int *b;
          a=10; b=&a;
          return f(&a,&b);
        }
        int f(int *p, int**pp) {
          return *p + **pp;
        }"
try 12 "int main() {int a; int *p; p=&a; *p=12; return a;}"
try 1 "int *main(int *a, int **b){int *p; 1;} int**func(int ***********a){&(a); &(*(a)); &**(**a);}"
try 1 "int *f(){int a; return &a;} int main(){int *a; a=f(); return 1;}"
try 1 "int* f(int *a){return a;} int main(){int x; int *y; y=f(&x); return &x==y;}"
try 8 "int **p; p=0; p++; ++p; return p-1;"
try 4 "int *p; int a; p=0; a=2; p=p+a; p--; --p; p=p+0; return 1+p;"
try 4 "int main(){int *p; alloc4(&p,1,2,4,8); *p=0; *(p+1)=0; return *(p+2);}"
try 36 "int a; int *p; return sizeof(a)+sizeof(p)+sizeof(1)+sizeof(&p)+sizeof(int)+sizeof(int*);"
try 40 "int a[2*5]; return sizeof(a);"
try 32 "return sizeof(int*[2*2]);"
try 8 "int a; return sizeof(1&&1==1>1)+sizeof(a=1);"
try 1 "int a[4]; return a==&a;"
try 4 "int a[4]; *a=2; *(a+1)=4; *(a+2)=8; return *(1+a);"
try 4 "int a[4]; a[0]=2; a[1]=4; a[2]=8; return a[1];"
try 8 "int a[4]; a[3]=8; (1,2)[a+1];"
try 6 "int func(int *a){return a[0]+a[1]+a[2];}
       int main(){int a[4]; a[0]=1; a[1]=2; a[2]=3; return func(a);}"

try $ER "int a; *a;"
try $ER "int *a; **a;"
try $ER "*1;"
try $ER "*1=0;"
try $ER "&1;"
try $ER "int a; *a;"
try $ER "int a; *a=0;"
try $ER "int a; & &a;"
try $WR "int *a; a=1;"
try $WR "int a; int*b; a=b;"
try $WR "int a; int*b; b=a;"
try $WR "int a; int b; a=&b;"
try $WR "int*a; int**b; a=b;"
try $WR "int *f(){int a; return &a;} int main(){int a; a=f();}"
try $ER "int *p; p+p;"
try $ER "int *p; p-p;"
try $ER "int *p; 1-p;"
try $ER "int *p; return sizeof(**p);"
try $ER "int a[4]; a=1;"

try 5 "int x; int y[4]; int*p; int main(){x=1; y[1]=2; p=y+1; return x+y[1]+*p;}"
try 3 "int x; int y[4]; int main(){int x; int y; x=1; y=2; return x+y;}"
try 6 "char c; char d; c=4;d=2;return c+d;"
try 6 "char s[4]; int main(){char*p; p=s; *p++=2; *p++=4; return s[0]+s[1];}"
try 7 'char buf[20]; strcpy(buf,"abc"); return printf("%s%d\n", buf, 123);'
try 66 "return 'A'+1;"

try $ER "int x; int x[4]; int main(){}"
try $ER '"ABC"=1;'

rm -f $EXE $EXE.s
echo "test: OK"