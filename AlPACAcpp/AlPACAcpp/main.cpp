/* main.cpp
 * -------------
 * Implementation of EchoServer using the C++ wrapper for AlPACA. */


#include <iostream>
#include "alpaca/alpaca.hpp"

using namespace std;

class EchoServer : public AlpacaServer {
public:
    virtual int serverFuncJoin(AlpacaConnection *connection, int func, void *arg) {
        connection->writeString(
            "=========================================================================\n"
            "Welcome to echoserver.  Everything you type will be echoed by the server.\n"
            "=========================================================================\n");
        return 1;
    }
    
    virtual int serverFuncLeave(AlpacaConnection *connection, int func, void *arg) {
        connection->writeString("==================== Goodbye, world. ====================\n");
        cout << "Closing a connection...\n";
        return 1;
    }
    
    virtual int serverFuncRead(AlpacaConnection *connection, int func, void *arg) {
        connection->writeString("hello\n");
        return 0;
    }
    
    virtual int serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg) {
        connection->writeString("alpaca> ");
        return 0;
    }
    
};


int main(int argc, const char * argv[])
{
    cout << "Instantiating server.\n";
    EchoServer *server = new EchoServer();
    cout << "Connecting to server.\n";
    if (server->connect(4096) == 2)
        return 2;
    
    cout << "Waiting...\n";
    server->wait();
    
    cout << "Disconnecting server.\n";
    server->disconnect();
    
    
    return 0;
}