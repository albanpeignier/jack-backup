#ifndef _COMMON_JACK_PORT_H
#define _COMMON_JACK_PORT_H

#include <jack/jack.h>

void jack_port_make_standard(jack_client_t *client, 
			     jack_port_t **ports, int n, int output);

int jack_port_connect_named(jack_client_t *client, 
		      const char *src, const char *dst);

int jack_port_disconnect_named(jack_client_t *client, 
			 const char *src, const char *dst);

int jack_port_is_connected_p(jack_client_t *j, 
			     const char *l, const char *r);

void jack_port_clear_all_connections(jack_client_t *j, 
				     const char *l);

#endif
