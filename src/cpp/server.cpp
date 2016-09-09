/* server.cpp
 * -------------
 * Implementation of AlpacaServer, the wrapper class for al_server_t. */


#include "alpaca/server.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;

AlpacaServer::AlpacaServer() {
    this->server = nullptr;
}

AlpacaServer::~AlpacaServer() {
    this->close();
}

int AlpacaServer::open(int port, unsigned long int flags) {
    if (this->server != nullptr)
        return 1;
    
    this->server = al_server_new(port, flags);
    this->server->cpp_wrapper = this;
    if (!al_server_start(this->server)) {
        fprintf (stderr, "Server failed to start.\n");
        this->close();
        return 2;
    }

    al_server_func_set(this->server, AL_SERVER_FUNC_JOIN,       this->_serverFuncJoin);
    al_server_func_set(this->server, AL_SERVER_FUNC_LEAVE,      this->_serverFuncLeave);
    al_server_func_set(this->server, AL_SERVER_FUNC_READ,       this->_serverFuncRead);
    al_server_func_set(this->server, AL_SERVER_FUNC_PRE_WRITE,  this->_serverFuncPreWrite);
    al_server_func_set(this->server, AL_SERVER_FUNC_MAX,        this->_serverFuncMax);
    
    return 0;
}

int AlpacaServer::close() {
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
        cout << "Server is connected with " << this->numConnections() << " connections.\n";
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

int AlpacaServer::broadcastGlobalMessage(char *string) {
//    for (unordered_map<al_connection_t *, AlpacaConnection *>::iterator connection = this->connections.begin();
//        connection != this->connections.end(); connection++)
//    {
//        if (connection->second->flags() & AL_CONNECTION_WROTE) {
//            connection->second->writeString("\r\n");
//        }
//    }
    cout << "Broadcasting global message:\t" << string << endl;
    int return_value = al_server_write_string(this->server, string);
    al_server_write_string(this->server, "\r\n");
    return return_value;
}

size_t AlpacaServer::numConnections() {
    return this->connections.size();
}

int AlpacaServer::disconnectClient(AlpacaConnection *connection) {
    // Results in calling _funcServerLeave(), and popConnection()
    //      Removes entry from this->connections and frees the memory al_connection_t* points to.
    int return_value = al_connection_free(connection->getPointer());
    
    // Delete the AlpacaConnection wrapper class instance.
    delete connection;
    
    return return_value;
}

int AlpacaServer::popConnection(AlpacaConnection *connection) {
    unordered_map<al_connection_t *, AlpacaConnection *>::const_iterator find = this->connections.find(connection->getPointer());
    if (find != this->connections.end())
        this->connections.erase(find);

    return 0;
}

int AlpacaServer::popConnection(al_connection_t *connection) {
    unordered_map<al_connection_t *, AlpacaConnection *>::const_iterator find = this->connections.find(connection);
    if (find != this->connections.end())
        this->connections.erase(find);
    
    return 0;
}

/* Hooks for various server events.  In order to be used, these should be overloaded in an inherited class. */
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
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    int start = this_ptr->numConnections();
    
    /* Build an AlpacaConnection wrapper object and record the mapping from *connection to it. */
    this_ptr->connections[connection] = new AlpacaConnection(connection);
    cout << "User joined: \t" << start << "->" << this_ptr->numConnections() << endl;
    
    return this_ptr->serverFuncJoin(this_ptr->connections[connection], func, arg);
}

int AlpacaServer::_serverFuncLeave(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    int start = this_ptr->numConnections();
    
    /* Remove the connection from the server's list of connections.
       NOTE: It is assumed that the connection itself is cleaned up elsewhere.  Ideally, that cleanup code will call this
             member function.  Most likely case: al_connection_free() will have called _serverFuncLeave().
     */
    int return_code = this_ptr->serverFuncLeave(this_ptr->connections[connection], func, arg);
    this_ptr->popConnection(connection);
    cout << "User left:\t\t" << start << "->" << this_ptr->numConnections() << endl;
    
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
    if (!al_server_is_quitting(this_ptr->server))
        return this_ptr->serverFuncPreWrite(this_ptr->connections[connection], func, arg);
    else
        return 1;
}

int AlpacaServer::_serverFuncMax(al_server_t *this_server, al_connection_t *connection, int func, void *arg)
{
    AlpacaServer *this_ptr = reinterpret_cast <AlpacaServer *>(this_server->cpp_wrapper);
    return this_ptr->serverFuncMax(this_ptr->connections[connection], func, arg);
}