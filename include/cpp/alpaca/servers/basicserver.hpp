/* servers/basicserver.hpp
 * -------------
 * Implementation for BasicServer.  Serves as an example server with some basic functionality. */

#ifndef __ALPACA_CPP_SERVERS_BASICSERVER_HPP
#define __ALPACA_CPP_SERVERS_BASICSERVER_HPP

extern "C" {
    #include "alpaca/alpaca.h"
}

#include "alpaca/connections.hpp"
#include "alpaca/server.hpp"
#include "alpaca/servers/emptyserver.hpp"


class BasicServer : public EmptyServer
{
public:
    virtual int serverFuncRead(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg);
};

#endif