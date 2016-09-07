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

bool AlpacaConnection::equals(al_connection_t *rhs) {
    return this->connection == rhs;
}