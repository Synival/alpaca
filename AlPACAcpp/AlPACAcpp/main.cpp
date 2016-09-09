/* main.cpp
 * -------------
 * Implementation of EchoServer using the C++ wrapper for AlPACA. */


#include <iostream>
#include "alpaca/alpaca.hpp"
#include "alpaca/servers/basicserver.hpp"

extern "C" {
    #include "alpaca/read.h"
}

using namespace std;


class EchoServer : public BasicServer
{
public:
    virtual int serverFuncJoin(AlpacaConnection *connection, int func, void *arg) {
        connection->writeString("========= Hello, and welcome to the server! =========\n");
        return 1;
    }
    
    virtual int serverFuncRead(AlpacaConnection *connection, int func, void *arg) {
        /* read lines until we can't anymore. */
        char buf[256], echo_string[258];
        while (al_read_line (buf, sizeof (buf), reinterpret_cast<al_func_read_t *>(arg))) {
            /* We match input against the following:
             1. If we get a blank line, skip it.
             2. If we get "shutdown", shut down the server and quit.
             3. If we get "disconnect", close only this client's connection to the server.
             4. If we get "shout ___", broadcast the "___" message to everyone on the server (and the server).
             5. Otherwise, echo the input back to the user that typed it.
             */
            if (buf[0] == '\0')                                             connection->connectionWrote();
            else if (strcmp(buf, "shutdown") == 0)                          return this->disconnect();
            else if (strcmp(buf, "disconnect") == 0)                        return this->disconnectClient(connection);
            else if (strlen(buf) > 6 && strncmp("shout ", buf, 6) == 0)     this->broadcastGlobalMessage(buf + 6);
            else {
                strcpy(echo_string, buf);
                connection->writeString( strcat(echo_string, "\r\n"));
            }
        }
        return 0;
    }
};

int main(int argc, const char * argv[])
{
    EchoServer *server = new EchoServer();
    if (server->connect(4096) == 2)
        return 2;
    
    cout << "Server is live.\n";
    server->wait();
    
    server->disconnect();
    cout << "Server has shut down.\n";
    
    return 0;
}