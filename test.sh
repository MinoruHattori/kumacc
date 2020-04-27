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

assert 0 0
assert 42 42
assert 32 '9+25-2'
assert 96 ' 44 + 58 - 6'

echo OK