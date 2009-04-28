#ifndef _COMMON_SIGNAL_INTERLEAVE_C
#define _COMMON_SIGNAL_INTERLEAVE_C

#include "signal-interleave.h"

/* f = number of frames, c = number of channels */

void
signal_interleave ( float *dst , const float *src , int f , int c ) 
{
  int i , k = 0 ;
  for ( i = 0 ; i < f ; i++ ) {
    int j ;
    for ( j = 0 ; j < c ; j++ ) {
      dst[k++] = src[(j*f)+i] ;
    }
  }
}

void
signal_uninterleave ( float *dst , const float *src , int f , int c ) 
{
  int i , k = 0 ;
  for ( i = 0 ; i < f ; i++ ) {
    int j ;
    for ( j = 0 ; j < c ; j++ ) {
      dst[(j*f)+i] = src[k++] ;
    }
  }
}

void
signal_interleave_to ( float *dst , const float **src , int f , int c ) 
{
  int i , k = 0 ;
  for ( i = 0 ; i < f ; i++ ) {
    int j ;
    for ( j = 0 ; j < c ; j++ ) {
      dst[k++] = src[j][i] ;
    }
  }
}

void
signal_uninterleave_to ( float **dst , const float *src , int f , int c ) 
{
  int i , k = 0 ;
  for ( i = 0 ; i < f ; i++ ) {
    int j ;
    for ( j = 0 ; j < c ; j++ ) {
      dst[j][i] = src[k++] ;
    }
  }
}

#endif
