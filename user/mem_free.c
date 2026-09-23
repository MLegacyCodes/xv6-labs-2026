#include "kernel/types.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  int total;
  int available;
  char *p;

  total = 0;
  available = 1;

  while (available == 1) {
    p = sbrk(PGSIZE); //asks kernel to get another page

    if (p == (char *)-1) {
      available = 0; //indicates memory has been filled
    } else {
      total = total + PGSIZE; 
    }
  }

  sbrk(-total); //kernel gets the allocated memory back

  printf("%d bytes available (%d pages)\n", total, total / PGSIZE);
  exit(0);
}
