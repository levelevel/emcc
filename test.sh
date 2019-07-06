#!/bin/bash
set -u
ulimit -c unlimited

EXE=tmp/test
AFLAGS="-g -no-pie"
CFLAGS="-D_9cc"
ER=Error
WR=Warning

rm -f $EXE.log

GDB=""
test_er_only=0
cnt=0

test_src() {
  src=$1
  src2=./test_src/extern.c
  EXE2=tmp/test_src
  rm -f $EXE2 $EXE2.log

  gcc -g $src $src2 -o $EXE2 > $EXE2.gcc.log 2>&1
  ./$EXE2          >> $EXE2.gcc.log
  if [ $? -eq 0 ]; then
    echo "gcc TEST OK    : $src"
  else
    egrep "error|9cc" $EXE2.gcc.log
    echo "gcc TEST FAIL! : $src"
    echo "see more information: $EXE2.gcc.log"
    exit 1;
  fi

  cpp $CFLAGS $src | grep -v "^#" > $EXE2.c
  ./9cc $src2 > ${EXE2}e.s
  ./9cc $EXE2.c 2>&1 > $EXE2.s | tee -a $EXE2.log | grep "9cc:Error" > $EXE2.err
  if [ $? -eq 0 ]; then
    cat $EXE2.log
    exit 1
  fi

  gcc $AFLAGS -o $EXE2 $EXE2.s ${EXE2}e.s
  
  ./$EXE2 >> $EXE2.log
  if [ $? -eq 0 ]; then
    echo "9cc TEST OK    : $src"
  else
    tail $EXE2.log
    echo "9cc TEST FAIL! : $src"
    exit 1;
  fi
}

try() {
  expected="$1"
  input="$2"
  let cnt++
  rm -f $EXE

  echo "=== test[$cnt] ================================" >> $EXE.log
  if [ $expected == $ER -o $expected == $WR ]; then
    #ErrorまたはWorningが出力されるのが期待値
    ./9cc -s "$input" 2>&1 > $EXE.s | tee -a $EXE.log | grep "9cc:$expected" > $EXE.err
    if [ $? -eq 0 ]; then
      actual=$expected
    else
      actual="Normal End"
    fi
    if [ $test_er_only -ne 0 ]; then cat $EXE.err; fi
    rm -f $EXE.err
  else
    if [ $test_er_only -ne 0 ]; then return; fi
    ./9cc -s "$input" > $EXE.s
    gcc $AFLAGS -o $EXE $EXE.s
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
  ./9cc -s "$*" > $EXE.s
  if [ $? != 0 ]; then exit 1; fi
  cat -n $EXE.s
  gcc $AFLAGS -o $EXE $EXE.s
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

test_src test_src/expr.c

try $ER "42"
try $ER "10 + / 2;"
try $ER "main(){}"
try $ER "int main(a){}"
try $ER "int main(1){}"
try $ER "int main(int a+1){}"
try $ER "int main(int a,){}"
try $ER "a;"
try $ER "int a; int a;"
#try $ER "int (a);"   #direct_declaratorの正しい実装によりOKとなった
try $ER "int &a;"
try $ER "int +a;"
try $ER "1++;"
try $ER "--1;"
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
try $ER "int *p; char*q; p-q;"
try $ER "int *p; 1-p;"
try $ER "int *p; return sizeof(**p);"
try $ER "int a[4]; a=1;"
try $ER "char *argv[];"
try $ER "char *argv[0];"
try $ER "int a; char *argv[a];"
try $ER "char  char a;"
try $ER "short short a;"
try $ER "int   int a;"
try $ER "long long long a;"
try $ER "void  void *p;"
try $ER "signed void *p;"
try $ER "unsigned void *p;"
try $ER "void a;"
try $ER "signed unsigned a;"
try $ER "ststic ststic int a;"
try $ER "extern extern int a;"
try $ER "extern ststic int a;"
try $ER "extern int x; extern int*x;"
try $ER "int main(){extern int x; extern int*x;}"
try $ER "static extern func(){} int main(){}"
#try $ER "extern int func(); extern int*func();"
try $ER "sizeof(static int);"
try $ER "sizeof(extern int);"
try $ER "int main(...){}"
try $ER "int main(int argc, ..., char *argv[]){}"
try $ER "int main(int argc, void){}"
try $ER "int func(int , void); int main(){}"
try $ER "int func(int a, char *a); int main(){}"
try $WR "int a = func();"
try $ER "int func(){} int func(){} int main(){}"
try $ER "int func     int func(){} int main(){}"
try $ER "int func(){} int func     int main(){}"
try $ER "int func(); *func();"
try $ER "int func(); int (*fp)()=func; *fp();"
try $ER "int func(); func()();"
try $ER "int func(); func()[];"
try $ER "int a; a();"
try $ER "int a; int main(){a();}"
try $ER "break;"
try $ER "continue;"
#LVALUE
try $ER "int a,b; (1?a:b)=0;"
try $ER "int a; a||1=0;"
try $ER "int a; a&&1=0;"
try $ER "int a; a^1=0;"
try $ER "int a; a|1=0;"
try $ER "int a; a&1=0;"
try $ER "int a; a==1=0;"
try $ER "int a; a>1=0;"
try $ER "int a; a+1+=0;"
try $ER "int a; a*1-=0;"
try $ER "int *a; (char*)a=0;"

try $ER "int a; a&1=0;"
try $ER "int a; a|1=0;"
try $ER "int a; a^1=0;"
try $ER "int a; ~a=0;"
try $ER "int a; a>>1=0;"
try $ER "int a; a<<1=0;"

try $ER "int*p; p&1;"
try $ER "int*p; p|1;"
try $ER "int*p; p^1;"
try $ER "int*p; ~p;"
try $ER "int*p; p>>1;"
try $ER "int*p; p<<1;"

try $WR "void main(){return 1;}"
try $WR "int main(){return;}"
try $WR "int main(){char*p=0; return p;}"
try $ER "goto;"
try $ER "goto L1;"
try $ER "L1:; L1:;"
try $ER "case 1: ;"
try $ER "default 1: ;"
try $ER "switch(1){case 1:; case 1:;}"
try $ER "switch(1){default:; default:;}"
try $ER "int e; enum ABC{A,B,C} e;"
try $ER "enum ABC{A,B,C} e; int e;"
try $ER "enum ABC{A,B,A} e;"
try $ER "enum ABC{A,B,C} e; enum ABC{X,Y,Z} x;"
try $ER "enum ABC{A,B,C} e; enum XYZ{A,Y,Z} x;"
try $ER "enum ABC{A,B,C}; enum ABC{P,Q,R} e;"
try $ER "int main(void); int main(int x){return 1:}"
try $ER "int main(void); int main(int){return 1:}"
try $ER "int main(void); int main(int x);"
try $ER "void main(void){} void main(int);"
try $ER "int main(int); int main(int, ...);"
try $ER "int main(int); int main(){return 1;};"

#多次元配列
try $ER "int a[][3]={{1,2,3},{11,12,13}; return a[1][2]}"

try $ER "int x; int x[4]; int main(){}"
try $ER '"ABC"=1;'
try $ER "char p[3]=1;"
try $ER 'int a[]="ABC";'
try $ER 'char *p[]="ABC";'
try $ER "int a; a[1];"

#rm -f $EXE $EXE.s
echo "test: OK"