#!/bin/bash
set -u

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
  rm -f $EXE

  cpp $CFLAGS $src | grep -v "^#" > $EXE.c
  ./9cc $EXE.c 2>&1 > $EXE.s | tee -a $EXE.log | grep "9cc:" > $EXE.err
  if [ $? -eq 0 ]; then
    cat $EXE.log
    exit 1
  fi

  gcc $AFLAGS -o $EXE $EXE.s
  
  ./$EXE
  if [ $? -eq 0 ]; then
    echo "TEST OK    : $src"
  else
    echo "TEST FAIL! : $src"
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
try $ER "int (a);"
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
try $ER "int *p; p-p;"
try $ER "int *p; 1-p;"
try $ER "int *p; return sizeof(**p);"
try $ER "int a[4]; a=1;"
try $ER "char *argv[];"
try $ER "char *argv[0];"
try $ER "int a; char *argv[a];"


#多次元配列
try $ER "int a[][3]={{1,2,3},{11,12,13}; return a[1][2]}"

try $ER "int x; int x[4]; int main(){}"
try $ER '"ABC"=1;'
try $ER "char p[3]=1;"
try $ER 'int a[]="ABC";'
try $ER 'char *p[]="ABC";'

rm -f $EXE $EXE.s
echo "test: OK"