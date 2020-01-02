#!/bin/bash
set -u
ulimit -c unlimited

EMCC=emcc
TESTDIR=./tmp
EXE=$TESTDIR/test
AFLAGS="-g -no-pie"
CPPFLAG="-D_emcc"
CFLAGS="$CPPFLAG -I./include -std=c11 -pedantic-errors "

rm -f $EXE.log

GDB=""
cnt=0

test_src() {
  src=$1
  src2=./test_src/extern.c
  EXE2=${EXE}_src
  rm -f $EXE2 $EXE2.log

# gcc $CFLAGS -g $src $src2 -o $EXE2 > $EXE2.gcc.log 2>&1
  gcc -g $src $src2 -o $EXE2 > $EXE2.gcc.log 2>&1
  ./$EXE2          >> $EXE2.gcc.log
  if [ $? -eq 0 ]; then
    echo "gcc TEST OK    : $src"
  else
    egrep "error|$EMCC" $EXE2.gcc.log
    echo "gcc TEST FAIL! : $src"
    echo "see more information: $EXE2.gcc.log"
    exit 1;
  fi

#  cpp -P $CFLAGS $src > $EXE2.c
  cpp $CFLAGS $src > $EXE2.c
  ./$EMCC $src2 > ${EXE2}e.s
  ./$EMCC $EXE2.c 2>&1 > $EXE2.s | tee -a $EXE2.log | grep "$EMCC:Error" > $EXE2.err
  if [ $? -eq 0 ]; then
    cat $EXE2.log
    exit 1
  fi

  gcc $AFLAGS -o $EXE2 $EXE2.s ${EXE2}e.s
  
  ./$EXE2 >> $EXE2.log
  if [ $? -eq 0 ]; then
    echo "$EMCC TEST OK    : $src"
  else
    tail $EXE2.log
    echo "$EMCC TEST FAIL! : $src"
    exit 1;
  fi
}

try1() {
  rm -f $EXE
  make -s $EMCC
  ./$EMCC "$@" > $EXE.s
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
  emcpp2)
    TESTDIR=./tmp2
    EMCC=./$1
    EXE=$TESTDIR/test;;
  -gdb) GDB=gdb;;
  *) try1 "$@";;
  esac
  shift
done

if [ ! -e $TESTDIR ]; then md $TESTDIR; fi

#テストプログラムをコンパイルする
test_src test_src/expr.c
./$EMCC -test > /dev/null 2> $TESTDIR/${EMCC}_test.log
tail -4 $TESTDIR/${EMCC}_test.log

echo "test: OK"