#ifndef _COMMON_JACK_CLIENT_C
#define _COMMON_JACK_CLIENT_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include "failure.h"
#include "jack-client.h"
#include "print.h"

void
jack_client_minimal_error_handler ( const char *desc )
{
  eprintf ( "jack error: %s\n" , desc ) ;
}

void
jack_client_minimal_shutdown_handler ( void *arg )
{
  eprintf ( "jack shutdown\n") ;
  FAILURE ;
}

jack_client_t * 
jack_client_unique ( const char *name )
{
  int n = (int) getpid () ;
  char uniq[64] ;
  snprintf ( uniq , 64 , "%s-%d" , name , n ) ;
  jack_client_t *client = jack_client_new ( uniq ) ;
  if ( ! client ) {
    eprintf ( "jack_client_new() failed: %s\n" , uniq ) ;
    FAILURE ;
  }
  return client ;
}

int 
jack_client_activate ( jack_client_t *client )
{
  int err = jack_activate ( client ) ;
  if ( err ) {
    eprintf ( "jack_activate() failed\n" ) ;
    FAILURE ;
  }
  return err ;
}

#endif
