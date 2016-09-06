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

class MyServer : public AlpacaServer {
public:
    virtual int serverFuncRead(al_connection_t *connection, int func, void *arg) {
        cout << "Derived function call! :)\n";
        return 0;
    }
};

int main(int argc, const char * argv[]) {
    // insert code here...
    
    cout << "Instantiating server.\n";
    MyServer *server = new MyServer();
    cout << "Connecting to server.\n";
    if (server->connect(4096) == 2)
        return 2;
    server->serverFuncRead(nullptr, 0, nullptr);
    server->_readFunc(nullptr, nullptr, 0, nullptr);
    
//    cout << "Waiting...\n";
//    server->wait();
    
    cout << "Disconnecting server.\n";
    server->disconnect();
    
    
    return 0;
}