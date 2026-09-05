#include "server/tcp_server.hpp"


int main(){
    TcpServer server("127.0.0.1" , 9000);
    
     return server.start() ? 0 : 1;
}