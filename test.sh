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

assert 0 'return 0;'
assert 42 'return 42;'
assert 32 'return 9+25-2;'
assert 96 'return  44 + 58 - 6;'
assert 29 'return 5+3*8;'
assert 15 'return 5*(9-6);'
assert 4 'return (1+3*2-1+2)/2;'
assert 4  'return (((3+2)*3)+1)/(3+1);'
assert 30 'return -14+44;'
assert 20 'return - -20;'
assert 20 'return - - +20;'

assert 0 'return -1==0;'
assert 1 'return 33==32+1;'
assert 1 'return 3!=2;'
assert 0 'return 23-1!=22;'

assert 1 'return 0<2;'
assert 0 'return 3<3;'
assert 0 'return 2<0;'
assert 1 'return -1<=0;'
assert 1 'return 2<=2;'
assert 0 'return 3<=2;'

assert 1 'return 2>0;'
assert 0 'return 3>3;'
assert 0 'return 2>3;'
assert 1 'return 2>=1;'
assert 1 'return 0>=0;'
assert 0 'return 2>=3;'

assert 1 'return 1; 3; 5;'
assert 3 '4; return 3; 9;'
assert 4 '1; 2; return 4;'

assert 5 'a=5; return a;'
assert 9 'a=4; z=5; return a+z;'
assert 8 'a=b=4; return a+b;'

echo OK
