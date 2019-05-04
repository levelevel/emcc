#!/bin/bash
set -u

EXE=tmp
AFLAGS=-g
ER=Error

try() {
  expected="$1"
  input="$2"

  if [ $expected == $ER ]; then
    ./9cc "$input" 2>&1 > $EXE.s | grep "9cc:Error" > /dev/null
    if [ $? -eq 0 ]; then
      actual=$ER
    else
      actual="Normal End"
    fi
  else
    ./9cc "$input" > $EXE.s
    gcc $AFLAGS -o $EXE $EXE.s func.o
    ./$EXE
    actual="$?"
  fi

  if [ "$actual" == "$expected" ]; then
    echo "# $input => $actual"
  else
    echo "#! $input => $expected expected, but got $actual"
    exit 1
  fi 
}

if [ $# -gt 0 ]; then
  GDB=""
  if [ "$1" == "-d" ]; then
    GDB=gdp
    shift
  fi
  make -s
  ./9cc "$*" > $EXE.s
  if [ $? != 0 ]; then exit 1; fi
  cat -n $EXE.s
  gcc $AFLAGS -o $EXE $EXE.s func.o
  $GDB ./$EXE
  echo $?
  exit
fi

try 42 "42;"
try $ER "42"
try 14 "10 + 2 * 3 - 4/2;"
try $ER "10 + * 2;"
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
try 3 "1,2,3;"
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

try 1 "int *main(int *a, int **b){int *p; 1;} int**func(int ***********a){a;}"

rm -f $EXE $EXE.s
echo "test: OK"