/* servers/emptyserver.cpp
 * -------------
 * Implementation for EmptyServer.  Not useful. */


#include "alpaca/servers/emptyserver.hpp"

int EmptyServer::serverFuncJoin(AlpacaConnection *connection, int func, void *arg) {
    return 1;
}

int EmptyServer::serverFuncLeave(AlpacaConnection *connection, int func, void *arg) {
    return 1;
}

int EmptyServer::serverFuncRead(AlpacaConnection *connection, int func, void *arg) {
    return 0;
}

int EmptyServer::serverFuncPreWrite(AlpacaConnection *connection, int func, void *arg) {
    return 0;
}
int EmptyServer::serverFuncMax(AlpacaConnection *connection, int func, void *arg) {
    return 0;
}