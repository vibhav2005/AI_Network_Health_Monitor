#ifndef TCP_SERVER
#define TCP_SERVER

#include <string>
#include <cstdint>
#include <winsock2.h>

class TcpServer {
    private:
        std::string address;
        std::uint16_t port;

    public:
        TcpServer(const std::string& address, std::uint16_t port);

        bool start();
        bool acceptClient(SOCKET listeningSocket, SOCKET& clientSocket, std::string& clientIp);
        bool receiveMessage(SOCKET clientSocket, std::string& message);
        bool parseMessage(const std::string& message, std::string& deviceId);
};

#endif
//tcp server hpp

