#include <stdio.h>

void func()
{
    int a,b,c,d,e,f,g,h,i,j;
    
    a = 10;
    b = 100;
    c = a + b;
    d = a - 2;
    b = a + b;
    if (a == 0) {
        e = b + a;
        f = a + 2;
    } else {
        g = a + 2;
        h = a * 2;
        i = b / 3;
    }

    for (i = 0; i < 100; i = i + 1) {
        h = a + b;
        g = a + b;
        a = a + b;
    }
}
