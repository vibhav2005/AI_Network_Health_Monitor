#ifndef TCP_SERVER
#define TCP_SERVER

#include <string>
#include <cstdint>

class TcpServer{
    private:
        std::string address;
        std::uint16_t port;

    public:
        TcpServer(const std::string& address, std::uint16_t port);
        bool start();

};

#endif
//tcp server hpp

