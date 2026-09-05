#ifndef CLIENT_SIM
#define CLIENT_SIM  

#include <string>
#include <cstdint>

class ClientSim{
    private:
        std:: string serverAddr;
        std:: uint16_t serverPort;
        std::string deviceId;

    public:
        ClientSim(const std::string& serverAddress,
                std::uint16_t serverPort,
                const std::string& deviceId);

        bool connectAndSend();
};

#endif//client sim hpp
