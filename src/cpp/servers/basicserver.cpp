/* servers/basicserver.cpp
 * -------------
 * Implementation for BasicServer.  Serves as an example server with some basic functionality. */


#include "alpaca/servers/basicserver.hpp"

int BasicServer::serverFuncRead(AlpacaConnection *connection, int func, void *arg) {
    /* read lines until we can't anymore. */
    char buf[256];
    while (al_read_line (buf, sizeof (buf), reinterpret_cast<al_func_read_t *>(arg))) {
        /* We match input against the following:
            1. If we get a blank line, skip it.
            2. If we get "shutdown", shut down the server and quit.
            3. If we get "disconnect", close only this client's connection to the server.
         */
        if (buf[0] == '\0')                        connection->connectionWrote();
        else if (strcmp(buf, "shutdown") == 0)     return this->close();
        else if (strcmp(buf, "disconnect") == 0)   return this->disconnectClient(connection);
    }
    return 0;
}

int BasicServer::serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg) {
    connection->writeString("> ");
    return 0;
}