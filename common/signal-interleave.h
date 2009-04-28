#ifndef _COMMON_SIGNAL_INTERLEAVE_H
#define _COMMON_SIGNAL_INTERLEAVE_H

void signal_interleave(float *dst, const float *src, int f, int c);
void signal_uninterleave(float *dst, const float *src, int f, int c);
void signal_interleave_to(float *dst, const float **src, int f, int c);
void signal_uninterleave_to(float **dst, const float *src, int f, int c);

#endif
