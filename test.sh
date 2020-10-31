#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 7 "1+2*3;"
assert 5 "1*2+3;"
assert 62 "1*2+3*4*5;"
assert 5 "10/2;"
assert 7 "2+10/2;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 2 '+(+2);'
assert 7 '+2+(+5);'
assert 10 '-10+20;'
assert 30 '+10+20;'
assert 10 '+20-10;'
assert 2 '-(-2);'
assert 15 '+10-(-5);'
assert 5 '+10-(+5);'
assert 5 '+10+(-5);'
assert 10 '-10+(+20);'
assert 10 '-10-(-20);'
assert 10 '+ + +10;'
assert 10 '+ - -10;'
assert 10 '- - +10;'

assert 1 '1 < 2;'
assert 0 '2 < 2;'
assert 0 '3 < 2;'
assert 0 '-1 < -2;'
assert 0 '-2 < -2;'
assert 1 '-3 < -2;'
assert 0 '(1+2) < 2;'
assert 0 '3 < (1+2);'
assert 1 '(1+2) < (1+5);'

assert 1 '1 <= 2;'
assert 1 '2 <= 2;'
assert 0 '3 <= 2;'
assert 0 '-1 <= -2;'
assert 1 '-2 <= -2;'
assert 1 '-3 <= -2;'
assert 0 '(1+2) <= 2;'
assert 1 '3 <= (1+2);'
assert 1 '(1+2) <= (1+5);'

assert 0 '1 > 2;'
assert 0 '2 > 2;'
assert 1 '3 > 2;'
assert 1 '-1 > -2;'
assert 0 '-2 > -2;'
assert 0 '-3 > -2;'
assert 1 '(1+2) > 2;'
assert 0 '3 > (1+2);'
assert 0 '(1+2) > (1+5);'

assert 0 '1 >= 2;'
assert 1 '2 >= 2;'
assert 1 '3 >= 2;'
assert 1 '-1 >= -2;'
assert 1 '-2 >= -2;'
assert 0 '-3 >= -2;'
assert 1 '(1+2) >= 2;'
assert 1 '3 >= (1+2);'
assert 0 '(1+2) >= (1+5);'

assert 1 "(2 >= 2);"
assert 1 "(2 <= 2);"
assert 1 "(2 == 2);"
assert 0 "(2 != 2);"
assert 1 "2 == 2;"
assert 1 "2 != 3;"
assert 0 "(2+1) == 2;"
assert 1 "2 != (2+1);"
assert 1 "(2+3) == (1+1+1+2);"

assert 42 "42;"
assert 4 "a=b=2; a+b;"
echo OK
