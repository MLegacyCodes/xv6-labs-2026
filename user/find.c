#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
basename(char *path)
{
  char *p;

  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

void
finddir(char *dir, char *target)
{
  char buf[512], *p;
  int fd, cfd;
  struct dirent de;
  struct stat st;

  if((fd = open(dir, 0)) < 0){
    printf("find: cannot open %s\n", dir);
    return;
  }

  if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof(buf)){
    printf("find: path too long\n");
    close(fd);
    return;
  }

  strcpy(buf, dir);
  p = buf + strlen(buf);
  *p++ = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0) continue;
    if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    if(strcmp(de.name, target) == 0)
      printf("%s\n", buf);

    if((cfd = open(buf, 0)) < 0) continue;
    if(fstat(cfd, &st) < 0){ close(cfd); continue; }
    if(st.type == T_DIR){
      close(cfd);
      finddir(buf, target);
    } else {
      close(cfd);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  int fd;
  struct stat st;

  if(argc != 3){
    printf("usage: find dir name\n");
    exit(1);
  }

  if((fd = open(argv[1], 0)) < 0){
    printf("find: cannot open %s\n", argv[1]);
    exit(1);
  }
  if(fstat(fd, &st) < 0){
    printf("find: cannot stat %s\n", argv[1]);
    close(fd);
    exit(1);
  }
  close(fd);

  if(st.type != T_DIR){
    if(strcmp(basename(argv[1]), argv[2]) == 0)
      printf("%s\n", argv[1]);
    exit(0);
  }

  finddir(argv[1], argv[2]);
  exit(0);
}
