/* alpaca.hpp
 * -------------
 * Definition AlpacaServer, the wrapper class for al_server_t. */

#ifndef __ALPACA_CPP_SERVER_HPP
#define __ALPACA_CPP_SERVER_HPP

#include <unordered_map>

extern "C" {
    #include "alpaca/alpaca.h"
}

#include "alpaca/connections.hpp"

class AlpacaServer {
private:
    al_server_t *server = nullptr;
    std::unordered_map<al_connection_t*, AlpacaConnection *> connections;

public:
    AlpacaServer();
    ~AlpacaServer();
    int open(int port, unsigned long int flags = 0);
    int close();
    bool isConnected();
    void printStatus();
    int wait();
    AlpacaConnection* getAlpacaConnection(al_connection_t *connection);
    int broadcastGlobalMessage(char *string);
    size_t numConnections();
    int disconnectClient(AlpacaConnection *connection);
    
    virtual int serverFuncJoin(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncLeave(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncRead(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg);
    virtual int serverFuncMax(AlpacaConnection *connection, int func, void *arg);
    
private:
    static AL_SERVER_FUNC(_serverFuncJoin);
    static AL_SERVER_FUNC(_serverFuncLeave);
    static AL_SERVER_FUNC(_serverFuncRead);
    static AL_SERVER_FUNC(_serverFuncPreWrite);
    static AL_SERVER_FUNC(_serverFuncMax);
    
    int popConnection(AlpacaConnection *connection);
    int popConnection(al_connection_t *connection);
};

#endif