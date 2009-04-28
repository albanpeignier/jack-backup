#ifndef _COMMON_SOUND_FILE_C
#define _COMMON_SOUND_FILE_C

#include <stdlib.h>
#include "failure.h"
#include "memory.h"
#include "sound-file.h"

SNDFILE * 	
xsf_open ( const char *path , int mode , SF_INFO *sfinfo )
{
  SNDFILE *s = NULL;

	while ( ! s ) {  
	  s = sf_open ( path , mode , sfinfo ) ;
	  if ( ! s ) {
		  sf_perror ( s ) ;
			fprintf ( stderr , "failed to open file : %s, retry in 5 seconds\n" , path ) ;
		  sleep(5);
		}
  }
  
  return s ;
}

void
xsf_handle_error ( SNDFILE *sndfile )
{
  int errnum = sf_error ( sndfile ) ;
  const char *errstr = sf_error_number ( errnum ) ;
  fprintf ( stderr , "libsndfile failed: %s\n" , errstr ) ;
  sleep(1);
  // FAILURE ;
}

sf_count_t	
xsf_read_float ( SNDFILE *sndfile , float *ptr , sf_count_t items )
{
  sf_count_t err = sf_read_float ( sndfile , ptr , items ) ;
  if ( err < 0 ) {
    xsf_handle_error ( sndfile ) ;
  }
  return err ;
}

sf_count_t	
xsf_write_float ( SNDFILE *sndfile , float *ptr , sf_count_t items )
{
  sf_count_t err = sf_write_float ( sndfile , ptr , items ) ;
  if ( err < 0 ) {
    xsf_handle_error ( sndfile ) ;
  }
  return err ;
}

/* Read the single channel sound file at `name' to a newly allocated
   array and store the size at `n'. */

float *
read_signal_file ( const char *name , int *n )
{
  SF_INFO sfi ;
  SNDFILE *sfp = sf_open ( name , SFM_READ , &sfi ) ;
  if ( ! sfp ) {
    fprintf ( stderr , "sf_open() failed\n" ) ;
    sf_perror ( sfp ) ;
    FAILURE ;
  }
  if ( sfi.channels != 1 ) {
    fprintf ( stderr , "illegal channel count: %d\n" , sfi.channels ) ;
    FAILURE ;
  }
  *n = sfi.frames ;
  float *data = xmalloc ( *n * sizeof(float) ) ;
  int err = sf_read_float ( sfp , data , *n ) ;
  if ( err == -1 ) {
    fprintf ( stderr , "sf_read_float() failed\n" ) ;
    sf_perror ( sfp ) ;
    FAILURE ;
  }
  return data ;  
}

void
write_signal_file ( const char *name , const float *data , int n )
{
  SF_INFO sfi ;
  sfi.channels = 1 ;
  sfi.samplerate = 44100 ;
  sfi.frames = 0 ;
  sfi.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT ;
  SNDFILE *sfp = sf_open ( name , SFM_WRITE , &sfi ) ;
  if ( ! sfp ) {
    sf_perror ( sfp ) ;
    FAILURE ;
  }
  int err = sf_write_float ( sfp , data , n ) ;
  if ( err == -1 ) {
    fprintf ( stderr , "sf_write_float() failed\n" ) ;
    sf_perror ( sfp ) ;
    FAILURE ;
  }
  sf_close ( sfp ) ;  
}

#endif
