#ifndef _COMMON_FILE_H
#define _COMMON_FILE_H

void *xrealloc(void *p, size_t size);
bool file_exists_p(const char *filename);
time_t stat_mtime(const char *filename);
size_t stat_size(const char *filename);
int xpipe(int filedes[2]);
ssize_t xwrite(int filedes, const void *buffer, size_t size);
ssize_t xread(int filedes, void *buffer, size_t size);

#endif
