//
//  main.cpp
//  AlPACAcpp
//
//  Created by Ryan Holben on 9/5/16.
//
//

#include <iostream>
#include "alpaca/alpaca.hpp"

using namespace std;

class EchoServer : public AlpacaServer {
public:
    virtual int serverFuncJoin(al_connection_t *connection, int func, void *arg) {
        al_connection_write_string(connection,
                                    "=========================================================================\n"
                                    "Welcome to echoserver.  Everything you type will be echoed by the server.\n"
                                    "=========================================================================\n"
                                    );
        return 1;
    }
    
    virtual int serverFuncLeave(al_connection_t *connection, int func, void *arg) {
        al_connection_write_string(connection, "==================== Goodbye, world. ====================\n");
        return 1;
    }
    
    virtual int serverFuncRead(al_connection_t *connection, int func, void *arg) {
        
        al_connection_write_string(connection, "hello\n");
        return 0;
    }
    
    virtual int serverFuncPreWrite(al_connection_t *connection, int func, void *arg) {
        al_connection_write_string(connection, "alpaca> ");
        return 0;
    }
    
};

int main(int argc, const char * argv[]) {
    // insert code here...
    
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