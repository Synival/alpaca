/* connections.cpp
 * -------------
 * Implementation of AlpacaConnection, the wrapper class for al_connection_t. */


#include <iostream>
#include "alpaca/connections.hpp"

using namespace std;

AlpacaConnection::AlpacaConnection(al_connection_t *connection) {
    this->connection = connection;
    cout << "Opened new connection.\n";
}

AlpacaConnection::~AlpacaConnection() {
    cout << "Closed a connection.\n";
}

bool AlpacaConnection::operator==(const AlpacaConnection &rhs) {
    return this->connection == rhs.connection;
}

bool AlpacaConnection::operator==(const al_connection_t *rhs) {
    return this->connection == rhs;
}

int AlpacaConnection::writeString(char *string) {
    return al_connection_write_string(this->connection, string);
}