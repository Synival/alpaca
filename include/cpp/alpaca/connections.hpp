/* connections.hpp
 * -------------
 * Definition AlpacaConnection, the wrapper class for al_connection_t. */

#ifndef __ALPACA_CPP_CONNECTIONS_HPP
#define __ALPACA_CPP_CONNECTIONS_HPP

extern "C" {
#include "alpaca/alpaca.h"
}

class AlpacaConnection {
//public:
private:
    al_connection_t *connection;
    
public:
    AlpacaConnection(al_connection_t *connection);
    ~AlpacaConnection();
    int disconnect();
    bool operator==(const AlpacaConnection &rhs);
    bool operator==(const al_connection_t *rhs);
    int writeString(char *string);
    al_flags_t flags();
    int connectionWrote();
    al_connection_t * getPointer();
};

#endif