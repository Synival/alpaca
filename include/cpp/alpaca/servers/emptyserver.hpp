/* servers/emptyserver.hpp
 * -------------
 * Definition for EmptyServer.  Empty template for your own server code. */

#ifndef __ALPACA_CPP_SERVERS_EMPTYSERVER_HPP
#define __ALPACA_CPP_SERVERS_EMPTYSERVER_HPP

extern "C" {
    #include "alpaca/alpaca.h"
}

#include "alpaca/server.hpp"

class EmptyServer : public AlpacaServer
{
public:
    virtual int serverFuncJoin(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncLeave(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncRead(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncMax(AlpacaConnection *connection, int func, void *arg);
};

#endif