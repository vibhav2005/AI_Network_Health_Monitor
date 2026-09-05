#include "client/client_sim.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>


ClientSim::ClientSim(const std::string& serverAddress,
                     std::uint16_t serverPort,
                     const std::string& deviceId)
    : serverAddr(serverAddress),
      serverPort(serverPort),
      deviceId(deviceId) {
}

bool ClientSim::connectAndSend() {
    if (deviceId.empty()) {
        std::cerr << "Device ID cannot be empty.\n";
        return false;
    }

    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create client socket.\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET,
                  serverAddr.c_str(),
                  &serverAddress.sin_addr) != 1) {
        std::cerr << "Invalid server address: " << serverAddr << '\n';
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    if (connect(clientSocket,
                reinterpret_cast<sockaddr*>(&serverAddress),
                sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to "
                  << serverAddr << ':' << serverPort << '\n';

        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    const std::string message = "DEVICE_ID=" + deviceId + "\n";

    int totalBytesSent = 0;
    const int messageLength = static_cast<int>(message.size());

    while (totalBytesSent < messageLength) {
        const int bytesSent = send(
            clientSocket,
            message.c_str() + totalBytesSent,
            messageLength - totalBytesSent,
            0
        );

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send device ID.\n";

            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        totalBytesSent += bytesSent;
    }

    std::cout << "Sent device ID: " << deviceId << '\n';

    closesocket(clientSocket);
    WSACleanup();

    return true;
}