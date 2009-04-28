#ifndef _COMMON_MEMORY_C
#define _COMMON_MEMORY_C

#include <stdlib.h>
#include <stdio.h>
#include "failure.h"
#include "memory.h"

void *
xmalloc ( size_t size ) 
{
  void *p = malloc ( size ) ;
  if ( p == NULL ) {
    fprintf ( stderr , "malloc() failed: %ld\n" , (long) size ) ;  
    FAILURE ;
  }
  return p ;
}

void *
xcalloc ( size_t count , size_t eltsize ) 
{
  void *p = calloc ( count , eltsize ) ;
  if ( p == NULL ) {
    perror ( "calloc() failed" ) ;  
    FAILURE ;
  }
  return p ;
}

void *
xrealloc ( void *p , size_t size ) 
{
  p = realloc ( p , size ) ;
  if ( p == NULL ) {
    perror ( "realloc() failed" ) ;
    FAILURE ;
  }
  return p ;
}

float *
fmalloc ( size_t n )
{
  float *d = xmalloc ( n * sizeof(float) ) ;
  size_t i ;
  for ( i = 0 ; i < n ; i++ ) {
    d[i] = 0.0 ;
  }
  return d ;
}
        
void
fmemset ( float *data , int n , float value )
{
  int i ;
  for ( i = 0 ; i < n ; i++ ) {
    data[i] = value ;
  }
}

#endif
