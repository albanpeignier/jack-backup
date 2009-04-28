#ifndef _COMMON_JACK_CLIENT_H
#define _COMMON_JACK_CLIENT_H

#include <jack/jack.h>

void jack_client_minimal_error_handler(const char *desc);

void jack_client_minimal_shutdown_handler(void *arg);

jack_client_t *jack_client_unique(const char *name);

int jack_client_activate(jack_client_t *client);

#endif
