#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data, int len);

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data, int len)
{
  int off = 0;

  for(int i = 0; fmt[i] != 0; i++){
    char c = fmt[i];

    if(c == 'i'){
      if(off + 4 > len){ printf("memdump: not enough data for 'i'\n"); return; }
      int v;
      memmove(&v, data + off, 4);
      printf("%d\n", v);
      off += 4;
    } else if(c == 'h'){
      if(off + 2 > len){ printf("memdump: not enough data for 'h'\n"); return; }
      short v;
      memmove(&v, data + off, 2);
      printf("%d\n", v);
      off += 2;
    } else if(c == 'c'){
      if(off + 1 > len){ printf("memdump: not enough data for 'c'\n"); return; }
      printf("%c\n", data[off]);
      off += 1;
    } else if(c == 'p'){
      if(off + 8 > len){ printf("memdump: not enough data for 'p'\n"); return; }
      unsigned long long v = 0;
      for(int k = 7; k >= 0; k--)
        v = (v << 8) | (unsigned char)data[off + k];
      char buf[17];
      buf[16] = 0;
      int pos = 16;
      if(v == 0){
        buf[--pos] = '0';
      } else {
        unsigned long long t = v;
        while(t > 0){
          int d = t & 0xf;
          buf[--pos] = d < 10 ? '0' + d : 'a' + d - 10;
          t >>= 4;
        }
      }
      printf("%s\n", &buf[pos]);
      off += 8;
    } else if(c == 's'){
      if(off + 8 > len){ printf("memdump: not enough data for 's'\n"); return; }
      unsigned long long v = 0;
      for(int k = 7; k >= 0; k--)
        v = (v << 8) | (unsigned char)data[off + k];
      printf("%s\n", (char*)v);
      off += 8;
    } else if(c == 'S'){
      char *p = data + off;
      int remaining = len - off;
      for(int k = 0; k < remaining && p[k] != 0; k++)
        printf("%c", p[k]);
      printf("\n");
      return;
    } else {
      printf("memdump: unknown format '");
      printf("%c", c);
      printf("'\n");
      return;
    }
  }
}
