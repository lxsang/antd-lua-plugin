#include <stdio.h>


typedef struct{
    char a;
    int b;
    short c;
    char d;
} inner_t;

typedef struct{
    char a;
    inner_t b;
    int c;
    char d;
} test_t;

char greet(const char* msg, float num, int sint, char c)
{
    printf("%s: '%f' '%d' '%c'\n", msg, num, sint, c);
    return 'A';
}

test_t test_struct(test_t data)
{
    printf("data is '%c' '%c' '%d' '%d' '%c' '%d' '%c'\n", data.a, data.b.a, data.b.b, data.b.c, data.b.d, data.c, data.d);
    return data;
}

void test_string(char* buff, const char* a)
{
    sprintf(buff,"you say %s", a);
    printf("%s\n", buff);
}