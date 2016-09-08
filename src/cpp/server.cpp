/* server.cpp
 * -------------
 * Implementation of AlpacaServer, the wrapper class for al_server_t. */


#include "alpaca/server.hpp"
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
    this->server->cpp_wrapper = this;
    if (!al_server_start(this->server)) {
        fprintf (stderr, "Server failed to start.\n");
        this->disconnect();
        return 2;
    }

    al_server_func_set(this->server, AL_SERVER_FUNC_JOIN,       this->_serverFuncJoin);
    al_server_func_set(this->server, AL_SERVER_FUNC_LEAVE,      this->_serverFuncLeave);
    al_server_func_set(this->server, AL_SERVER_FUNC_READ,       this->_serverFuncRead);
    al_server_func_set(this->server, AL_SERVER_FUNC_PRE_WRITE,  this->_serverFuncPreWrite);
    al_server_func_set(this->server, AL_SERVER_FUNC_MAX,        this->_serverFuncMax);
    
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

AlpacaConnection* AlpacaServer::getAlpacaConnection(al_connection_t *connection) {
    unordered_map<al_connection_t *, AlpacaConnection *>::const_iterator find = this->connections.find(connection);
    if (find == this->connections.end())
        return nullptr;
    else
        return find->second;
}


/* Hooks for various server events.  To be used, these should be overloaded in an inherited class. */
int AlpacaServer::serverFuncJoin(AlpacaConnection *connection, int func, void *arg) {
    cout << "This is an empty serverFuncJoin(), please write your own!\n";
    return 1;
}

int AlpacaServer::serverFuncLeave(AlpacaConnection *connection, int func, void *arg) {
    cout << "This is an empty serverFuncLeave(), please write your own!\n";
    return 1;
}

int AlpacaServer::serverFuncRead(AlpacaConnection *connection, int func, void *arg) {
    cout << "This is an empty readFunc(), please write your own!\n";
    return 0;
}

int AlpacaServer::serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg) {
    cout << "This is an empty serverFuncPreWrite(), please write your own!\n";
    return 0;
}

int AlpacaServer::serverFuncMax(AlpacaConnection *connection, int func, void *arg) {
    cout << "This is an empty serverFuncMax(), please write your own!\n";
    return 0;
}

/* Internal static class member functions which wrap the server hooks that an AlPACA server must implement. */
int AlpacaServer::_serverFuncJoin(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    // Build an AlpacaConnection wrapper object and record the mapping from *connection to it.
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    this_ptr->connections[connection] = new AlpacaConnection(connection);
    
    return this_ptr->serverFuncJoin(this_ptr->connections[connection], func, arg);
}

int AlpacaServer::_serverFuncLeave(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    int return_code = this_ptr->serverFuncLeave(this_ptr->connections[connection], func, arg);
    
    /* Attempt to remove the connection from this->connections.
       Delete the AlpacaConnection instance, then remove the container that pointed to it. */
    unordered_map<al_connection_t *, AlpacaConnection *>::const_iterator find = this_ptr->connections.find(connection);
    if (find != this_ptr->connections.end()) {
        delete find->second;
        this_ptr->connections.erase(find);
    }
    return return_code;
}

int AlpacaServer::_serverFuncRead(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    return this_ptr->serverFuncRead(this_ptr->connections[connection], func, arg);
}

int AlpacaServer::_serverFuncPreWrite(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    return this_ptr->serverFuncPreWrite(this_ptr->connections[connection], func, arg);
}

int AlpacaServer::_serverFuncMax(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    return this_ptr->serverFuncMax(this_ptr->connections[connection], func, arg);
}