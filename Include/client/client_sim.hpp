#ifndef CLIENT_SIM
#define CLIENT_SIM

#include "client/client_telemetry.hpp"

#include <string>
#include <cstdint>
#include <thread>
#include <winsock2.h>

class ClientSim {
    private:
        std::string serverAddr;
        std::uint16_t serverPort;
        std::string deviceId;
        clientTelemetry telemetry;
        bool connected;
        SOCKET clientSocket;
        std::thread telemetryThread;

        bool connectToServer();
        bool sendData(const std::string& payload);

    public:
        ClientSim(const std::string& serverAddress,
                  std::uint16_t serverPort,
                  const std::string& deviceId);
        ~ClientSim();

        bool generateTelemetry();
        std::string serializeTelemetryJson() const;
        bool sendTelemetry();
        void sendTelemetryStream(int delayMs = 500);
        bool connectAndSend();
        bool isConnected() const;
};

#endif // client sim hpp
