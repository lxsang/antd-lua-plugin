#include <stdio.h>
#include <stdlib.h>
char data[] = {'h', 'e','l', 'l', 'o'};

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

struct i2c_smbus_ioctl_data
{
  char read_write ;
  uint8_t command ;
  int size ;
  void *data ;
} ;
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

int test_struct_ptr(struct i2c_smbus_ioctl_data* data)
{
    printf("Hello\n");
    printf("rw %d cmd %d size %d %s\n", data->read_write, data->command, data->size, (char*)data->data);
    return 1;
}