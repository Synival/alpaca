/* connections.hpp
 * -------------
 * Definition AlpacaConnection, the wrapper class for al_connection_t. */

#ifndef __ALPACA_CPP_CONNECTIONS_HPP
#define __ALPACA_CPP_CONNECTIONS_HPP

extern "C" {
#include "alpaca/alpaca.h"
}

class AlpacaConnection {
private:
    al_connection_t *connection;
    
public:
    AlpacaConnection(al_connection_t *connection);
    ~AlpacaConnection();
    bool equals(al_connection_t *rhs);
};

#endif