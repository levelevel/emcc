#!/bin/bash
set -u
ulimit -c unlimited

EMCC=emcc
TESTDIR=./tmp
AFLAGS="-g -no-pie"
GDB=""

try1() {
  EXE=$TESTDIR/test1
  rm -f $EXE $EXE.log
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
  emcc2)
    TESTDIR=./tmp2
    EMCC=$1;;
  -gdb) GDB=gdb;;
  *) try1 "$@";;
  esac
  shift
done

if [ ! -e $TESTDIR ]; then mkdir $TESTDIR; fi

TESTEXEGCC=tmp/test_gcc
TESTEXEEMCC=tmp/test_emcc
make -s $TESTEXEGCC $TESTEXEEMCC

#gcc版テスト
$TESTEXEGCC > $TESTEXEGCC.log
if [ $? -eq 0 ]; then
  echo "### gcc TEST OK"
else
  egrep "error|$EMCC" $TESTEXEGCC.log
  echo "### gcc TEST FAIL!"
  echo "### see more information: $TESTEXEGCC.log"
  exit 1;
fi

#emcc版テスト
$TESTEXEEMCC > $TESTEXEEMCC.log
if [ $? -eq 0 ]; then
  echo "### $EMCC TEST OK"
else
  tail $TESTEXEEMCC.log
  echo "### $EMCC TEST FAIL!"
  exit 1;
fi

#セルフテスト
./$EMCC -test > /dev/null 2> $TESTDIR/${EMCC}.self_test.log
if [ $? -eq 0 ]; then
  tail -4 $TESTDIR/${EMCC}.self_test.log
  echo "### $EMCC SELF TEST OK"
else
  echo ...
  tail -4 $TESTDIR/${EMCC}.self_test.log
  echo "### $EMCC SELF TEST FAIL!"
  exit 1;
fi

echo "### test: OK"
exit 0
