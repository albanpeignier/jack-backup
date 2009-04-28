#ifndef _COMMON_MEMORY_H
#define _COMMON_MEMORY_H

#include <stddef.h>

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t eltsize);
void *xrealloc(void *p, size_t size);
float *fmalloc(size_t n);
void fmemset(float *data, int n, float value);

#endif
