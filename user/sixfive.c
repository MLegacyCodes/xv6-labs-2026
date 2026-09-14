#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *seps = " -\r\t\n./,";

void
flush(int num, int valid, int got)
{
  if(valid && got && (num % 5 == 0 || num % 6 == 0))
    printf("%d\n", num);
}

void
process(int fd)
{
  char buf[512];
  int n, num = 0, valid = 1, got = 0;

  while((n = read(fd, buf, sizeof(buf))) > 0){
    for(int i = 0; i < n; i++){
      char c = buf[i];
      if(c >= '0' && c <= '9'){
        if(valid){ num = num * 10 + (c - '0'); got = 1; }
      } else if(strchr(seps, c)){
        flush(num, valid, got);
        num = 0; valid = 1; got = 0;
      } else {
        valid = 0; num = 0; got = 0;
      }
    }
  }
  flush(num, valid, got);
}

int
main(int argc, char *argv[])
{
  if(argc < 2){ process(0); exit(0); }
  for(int i = 1; i < argc; i++){
    int fd = open(argv[i], O_RDONLY);
    if(fd < 0){ printf("sixfive: cannot open %s\n", argv[i]); continue; }
    process(fd);
    close(fd);
  }
  exit(0);
}
