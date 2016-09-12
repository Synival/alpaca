/* connections.cpp
 * -------------
 * Implementation of AlpacaConnection, the wrapper class for al_connection_t. */


//#include <iostream>
#include "alpaca/connections.hpp"
#include "alpaca/server.hpp"

//using namespace std;

AlpacaConnection::AlpacaConnection(al_connection_t *connection) {
    this->connection = connection;
}

AlpacaConnection::~AlpacaConnection() {
    this->disconnect();
}

int AlpacaConnection::disconnect() {
    if (this->connection == nullptr)
        return 1;
    
    this->connection = nullptr;
    return 0;
}

bool AlpacaConnection::operator==(const AlpacaConnection &rhs) {
    return this->connection == rhs.connection;
}

bool AlpacaConnection::operator==(const al_connection_t *rhs) {
    return this->connection == rhs;
}

int AlpacaConnection::writeString(const char *string) {
    return al_connection_write_string(this->connection, string);
}

al_flags_t AlpacaConnection::flags() {
    return this->connection->flags;
}

int AlpacaConnection::connectionWrote() {
    return al_connection_wrote(this->connection);
}
