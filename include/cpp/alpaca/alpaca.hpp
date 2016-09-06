/* alpaca.hpp
 * -------------
 * all C++ header files for alpaca. */


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
    
    virtual int serverFuncJoin(al_connection_t *connection, int func, void *arg);
    virtual int serverFuncLeave(al_connection_t *connection, int func, void *arg);
    virtual int serverFuncRead(al_connection_t *connection, int func, void *arg);
    virtual int serverFuncPreWrite(al_connection_t *connection, int func, void *arg);
    virtual int serverFuncMax(al_connection_t *connection, int func, void *arg);
    
private:
    static int _serverFuncJoin(al_server_t *this_server, al_connection_t *connection, int func, void *arg);
    static int _serverFuncLeave(al_server_t *this_server, al_connection_t *connection, int func, void *arg);
    static int _serverFuncRead(al_server_t *this_server, al_connection_t *connection, int func, void *arg);
    static int _serverFuncPreWrite(al_server_t *this_server, al_connection_t *connection, int func, void *arg);
    static int _serverFuncMax(al_server_t *this_server, al_connection_t *connection, int func, void *arg);
};
