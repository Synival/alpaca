/* alpaca.hpp
 * -------------
 * all C++ header files for alpaca. */

#include <stdio.h>

extern "C" {
    #include "alpaca/alpaca.h"
}

class AlpacaServer {
private:
    al_server_t *server = nullptr;

public:
    AlpacaServer();
    ~AlpacaServer();
    int connect(int port, unsigned long int flags = 0);
    int disconnect();
    bool isConnected();
    void printStatus();
    int wait();
//    virtual int serverFuncRead();
    virtual int serverFuncRead(al_connection_t *connection, int func, void *arg);
    
//private:
//public:
    static int _readFunc(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
    {
        AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->wrapper);
        return this_ptr->serverFuncRead(connection, func, arg);
//        return reinterpret_cast<AlpacaServer *>(this_server)->serverFuncRead(connection, func, arg);
    }
//    static AL_SERVER_FUNC(_readFunc) {return this->serverFuncRead(connection, func, arg);}
};
