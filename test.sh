#!/bin/bash
set -u

ASM=tmp

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > $ASM.s
  gcc -o $ASM $ASM.s func.o
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
  gcc -o $ASM $ASM.s func.o
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
try 0 "a=3;while(a)a=a-1;a;"
try 5 "for(a=0;a<5;a=a+1)a;return a;5;"
try 0 "if(0);while(0);"
try 0 "if(0)while(0)if(1)while(0);"
try 3 "a=3;for(;;)return a;"
try 5 "5; ;;;;;;;;"
try 5 "a = + - (+ - +5);"
try 7 "a=1;b=2; if(a+b>0){a=a+1;b=5;a+b;}"
try 6 "a=1;b=a++;c=++b;a+b+c;"
try 3 "a=2;b=a--;c=--b;a+b+c;"
try 55 "sum=0; for(i=10;i>0;i--) {sum=sum+i;} return sum;"
try 55 "sum=0; i=0; while(i<=10) {sum=sum+i; i++;} if(!!1)return sum;"
try 2 "a=0; if(0) a=1; else a=2; a;"
try 2 "a=0; if(0) {a=1;} else if (1) {a=2;} else {a=3;} a;"
try 1 "true=1&&1&&1;false=0&&1;true==1&&false==0;"
try 1 "a=1;b=0;true=a||b;false=b||0;true==1&&false==0;"
try 0 "ret=1;ret=foo(); bar(); ret;"
try 0 "a=2;ret=1;foo(); ret=bar(); ret;"

rm -f $ASM $ASM.s
echo "test: OK"