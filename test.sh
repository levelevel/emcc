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
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

./runtest

try 42 42
try 55 "100  - 50  + 5 "
try 14 "10 + 2 * 3 - 4/2"
try 5 "1*(2+3)"
try 3 "(((2+4)*1)/2)"
try 2 "+5%-(-3)"

echo "test: OK"