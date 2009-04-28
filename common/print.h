#ifndef _COMMON_PRINT_H
#define _COMMON_PRINT_H

#include <stdio.h>

#define eprintf(...) fprintf(stderr,__VA_ARGS__)

#ifdef DEBUG
  #define dprintf(...) fprintf(stderr,__VA_ARGS__)
#else
  #define dprintf(...) 
#endif

#endif
