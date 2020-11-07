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

assert 1 "foo=1;"
assert 5 "bar=2+3;"
assert 6 "foo=1; bar=2+3; (foo+bar);"

assert 42 "return 42;"
assert 5 "return 5; return 8;"
assert 5 "return(2+3);"
assert 6 "foo=1;bar=2+3;return (foo+bar);"

assert 1 "_Foo = 1;"
assert 5 "Bar_ = 2+3;"
assert 8 "foo_bar = 3+5;"
assert 13 "fn1=5; fn2=3+5; return (fn1+fn2);"

assert 20 "a = 10; if (1) a = 20; return a;"
assert 10 "a = 10; if (0) a = 20; return a;"
assert 200 "a = 20; if (1) a = a * 10; else a = a / 10; return a;"
assert 2 "a = 20; if (0) a = a * 10; else a = a / 10; return a;"

assert 0  "a = 5; while (a > 0) a = a - 1; return a;"
assert 64 "a = 1; while (a < 64) a = a * 2; return a;"
assert 1  "a = 1; while (a != 1) a = 10; return a;"
assert 10 "a=0; while (1) if (a < 10) a=a+1; else return a;"

assert 10 "for (a=0; a<10; a=a+1) 0; return a;"
assert 10 "for (a=0; a<10; ) a=a+1; return a;"
assert 10 "a=0; for (; a<10; a=a+1) 0; return a;"
assert 10 "a=0; for (; a<10; ) a=a+1; return a;"
assert 10 "a=10; for (; a<10; ) a=a+1; return a;"
assert 10 "a=0; for (;;) if (a < 10) a=a+1; else return a;"

assert 3 "{a=1;b=2;return(a+b);}"
assert 20 "a = 10; b = 20; if (1) { tmp = a; a = b; b = tmp; } return a;"
assert 39 "a = 390; b = 273; r = a; while (r) { q = a / b; r = a - b * q; a = b; b = r; } return a;"
assert 55 "result = 0; m = 0; n = 1; for (i = 1; i <= 10; i=i+1) { result = m+n; n=m; m=result; } return result;"

echo OK
