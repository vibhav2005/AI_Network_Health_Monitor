#include "server/tcp_server.hpp"
#include "D:/postgreSql/include/libpq-fe.h"
#include <iostream>

int main(){

    const char* conninfo =
        "host=localhost "
        "port=5432 "
        "dbname=Network_Monitor "
        "user=postgres "
        "password=1234";

    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) == CONNECTION_OK)
    {
        std::cout << "PostgreSQL connection successful!\n";
    }
    else
    {
        std::cerr << "Connection failed: "
                  << PQerrorMessage(conn);
    }

    PQfinish(conn);
}