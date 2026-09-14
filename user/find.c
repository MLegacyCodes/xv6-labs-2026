#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

int matchhere(char *, char *);
int matchstar(int, char *, char *);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do {
    if(matchhere(re, text))
      return 1;
  } while(*text++ != '\0');
  return 0;
}

int
matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text != '\0' && (re[0] == '.' || re[0] == *text))
    return matchhere(re+1, text+1);
  return 0;
}

int
matchstar(int c, char *re, char *text)
{
  do {
    if(matchhere(re, text))
      return 1;
  } while(*text != '\0' && (*text++ == c || c == '.'));
  return 0;
}

char *target;
char **exec_argv;
int exec_argc;
int exec_mode;

char*
basename(char *path)
{
  char *p;
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

void
handle(char *path)
{
  if(!exec_mode){
    printf("%s\n", path);
    return;
  }

  char *argv[MAXARG];
  int i;
  for(i = 0; i < exec_argc; i++)
    argv[i] = exec_argv[i];
  argv[i++] = path;
  argv[i] = 0;

  int pid = fork();
  if(pid < 0){
    printf("find: fork failed\n");
    return;
  }
  if(pid == 0){
    exec(argv[0], argv);
    printf("find: exec %s failed\n", argv[0]);
    exit(1);
  }
  wait(0);
}

void
finddir(char *dir)
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

    if(match(target, de.name))
      handle(buf);

    if((cfd = open(buf, 0)) < 0) continue;
    if(fstat(cfd, &st) < 0){ close(cfd); continue; }
    if(st.type == T_DIR){
      close(cfd);
      finddir(buf);
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

  if(argc < 3){
    printf("usage: find dir name [-exec cmd args...]\n");
    exit(1);
  }

  target = argv[2];

  if(argc > 3 && strcmp(argv[3], "-exec") == 0){
    exec_mode = 1;
    exec_argv = &argv[4];
    exec_argc = argc - 4;
    if(exec_argc < 1){
      printf("find: -exec needs a command\n");
      exit(1);
    }
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
    if(match(target, basename(argv[1])))
      handle(argv[1]);
    exit(0);
  }

  finddir(argv[1]);
  exit(0);
}
