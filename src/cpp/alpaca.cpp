/* wrapper.cpp
 * -------------
 * C++ class wrapper for AlPACA's C functionality. */

#include "alpaca/alpaca.hpp"
#include <iostream>

using namespace std;

AlpacaServer::AlpacaServer() {
    this->server = nullptr;
}

AlpacaServer::~AlpacaServer() {
    this->disconnect();
}

int AlpacaServer::connect(int port, unsigned long int flags) {
    if (this->server != nullptr)
        return 1;
    
    this->server = al_server_new(port, flags);
    this->server->wrapper = this;
    if (!al_server_start(this->server)) {
        fprintf (stderr, "Server failed to start.\n");
        this->disconnect();
        return 2;
    }

    al_server_func_set(this->server, AL_SERVER_FUNC_READ, this->_serverFuncRead);

    // al_server_wait (server);
    
    return 0;
}

int AlpacaServer::disconnect() {
    if (this->server == nullptr)
        return 1;
    
    al_server_free(this->server);
    this->server = nullptr;
    return 0;
}

bool AlpacaServer::isConnected() {
    return this->server != nullptr;
}

void AlpacaServer::printStatus() {
    if (this->isConnected())
        cout << "Server is connected.\n";
    else
        cout << "Server is disconnected.\n";
}

int AlpacaServer::wait() {
    return al_server_wait(this->server);
}

int AlpacaServer::serverFuncRead(al_connection_t *connection, int func, void *arg) {
    cout << "This is an empty readFunc(), please write your own!\n";
    return 0;
}

int AlpacaServer::_serverFuncRead(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->wrapper);
    return this_ptr->serverFuncRead(connection, func, arg);
}