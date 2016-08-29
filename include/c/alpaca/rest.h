/* rest.h
 * ------
 * RESTful API development tools. */

#ifndef __ALPACA_C_REST_H
#define __ALPACA_C_REST_H

#include "defs.h"

/* functions. */
int al_rest_init (al_server_t *server);

/* hooks and default functions. */
AL_SERVER_FUNC (al_rest_func_read);

#endif
