#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./kumacc "$input" > tmp.s || exit
  gcc -static -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 32 '9+25-2;'
assert 96 ' 44 + 58 - 6;'
assert 29 '5+3*8;'
assert 15 '5*(9-6);'
assert 4 '(1+3*2-1+2)/2;'
assert 4  '(((3+2)*3)+1)/(3+1);'
assert 30 '-14+44;'
assert 20 '- -20;'
assert 20 '- - +20;'

assert 0 '-1==0;'
assert 1 '33==32+1;'
assert 1 '3!=2;'
assert 0 '23-1!=22;'

assert 1 '0<2;'
assert 0 '3<3;'
assert 0 '2<0;'
assert 1 '-1<=0;'
assert 1 '2<=2;'
assert 0 '3<=2;'

assert 1 '2>0;'
assert 0 '3>3;'
assert 0 '2>3;'
assert 1 '2>=1;'
assert 1 '0>=0;'
assert 0 '2>=3;'

assert 5 '1; 3; 5;'

echo OK
