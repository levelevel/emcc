#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 42 42
try 55 "100  - 50  + 5 "
try 14 "10 + 2 * 3 - 4/2"
try 5 "1*(2+3)"
try 3 "(((2+4)*1)/2)"
try 2 "+5%-(-3)"
try 1 "1<2"
try 0 "5<5"
try 0 "100<99"
try 0 "1>2"
try 0 "5>5"
try 1 "100>99"
try 1 "1<=2"
try 1 "5<=5"
try 0 "100<=99"
try 0 "1>=2"
try 1 "5>=5"
try 1 "100>=99"
try 1 "4==4"
try 0 "4==(4+1)"
try 0 "4!=4"
try 1 "4 != (4+1)"

echo "test: OK"