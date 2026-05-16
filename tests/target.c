#include <stdio.h>
#include <stdlib.h>

static int add(int a, int b)
{
    return a + b;
}

static int multiply(int a, int b)
{
    int result = 0;
    for (int i = 0; i < b; i++)
        result = add(result, a);
    return result;
}

static int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

static int factorial(int n)
{
    if (n <= 1)
        return 1;
    return multiply(n, factorial(n - 1));
}

int main(void)
{
    printf("rupture test target\n");

    int a = add(3, 4);
    printf("add(3, 4) = %d\n", a);

    int m = multiply(6, 7);
    printf("multiply(6, 7) = %d\n", m);

    for (int i = 0; i < 10; i++) {
        printf("fib(%d) = %d\n", i, fib(i));
    }

    for (int i = 1; i <= 7; i++) {
        printf("factorial(%d) = %d\n", i, factorial(i));
    }

    printf("done\n");
    return 0;
}
