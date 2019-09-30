#!/bin/bash
set -u
ulimit -c unlimited

EXE=tmp/test
AFLAGS="-g -no-pie"
CFLAGS="-D_9cc -std=c11 -pedantic-errors "
ER=Error
WR=Warning

rm -f $EXE.log

GDB=""
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

try1() {
  rm -f $EXE
  make -s
  ./9cc "$@" > $EXE.s
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
  -gdb) GDB=gdb;;
  *) try1 "$@";;
  esac
  shift
done

test_src test_src/expr.c
./9cc -test > /dev/null 2> tmp/9cc_test.log
tail -4 tmp/9cc_test.log

#rm -f $EXE $EXE.s
echo "test: OK"