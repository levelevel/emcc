#!/bin/bash
set -u

ASM=tmp

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > $ASM.s
  gcc -o $ASM $ASM.s
  ./$ASM
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi 
}

if [ $# -gt 0 ]; then
  make -s
  ./9cc "$*" > $ASM.s
  if [ $? != 0 ]; then exit 1; fi
  cat -n $ASM.s
  gcc -o $ASM $ASM.s
  ./$ASM
  echo $?
  exit
fi

try 42 "42;"
try 14 "10 + 2 * 3 - 4/2;"
try 3 "(((2+4)*1)/2);"
try 2 "+5%-(-3);"
try 6 "(1<2) + (2>1) + (3<=3) + (4>=4) + (5==5) + (6!=6+1);"
try 0 "(2<1) + (1>2) + (4<=3) + (3>=4) + (5==5-1) + (6!=6);"
try 7 "a=1; b = 2 ;(c= -3); sum = a+b* -c; return sum;"
try 3 "a=2;if(1)if(a==2)a=a+1;a;"

rm -f $ASM $ASM.s
echo "test: OK"