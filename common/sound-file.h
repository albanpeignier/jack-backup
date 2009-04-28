#ifndef _COMMON_SOUND_FILE_H
#define _COMMON_SOUND_FILE_H

#include <sndfile.h>

SNDFILE *xsf_open(const char *path, int mode, SF_INFO *sfinfo);
void xsf_handle_error(SNDFILE *sndfile);
sf_count_t xsf_read_float(SNDFILE *sndfile, float *ptr, sf_count_t items);
sf_count_t xsf_write_float(SNDFILE *sndfile, float *ptr, sf_count_t items);
float *read_signal_file(const char *name, int *n);
void write_signal_file(const char *name, const float *data, int n);

#endif
