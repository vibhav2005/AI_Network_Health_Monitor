#include "server/tcp_server.hpp"

#include "winsock2.h"
#include "ws2tcpip.h"

#include <iostream>

TcpServer:: TcpServer(const std::string& address, std::uint16_t port):
    address(address),
    port(port){}


/*Start Windows networking
→ create socket
→ assign IP + port
→ listen
→ accept a client
→ receive message
→ validate message
→ close everything*/

bool TcpServer::start() {
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create listening socket.\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    if (inet_pton(AF_INET,
                  address.c_str(),
                  &serverAddress.sin_addr) != 1) {
        std::cerr << "Invalid server address: " << address << '\n';
        closesocket(listeningSocket);
        WSACleanup();
        return false;
    }

    if (bind(listeningSocket,
             reinterpret_cast<sockaddr*>(&serverAddress),
             sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind to " << address << ':' << port << '\n';
        closesocket(listeningSocket);
        WSACleanup();
        return false;
    }

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for incoming connections.\n";
        closesocket(listeningSocket);
        WSACleanup();
        return false;
    }

    std::cout << "Server listening on " << address << ':' << port<< '\n';

    while (true) {
        std::cout << "Waiting for a client connection...\n";

        sockaddr_in clientAddress{};
        int clientAddressLength = sizeof(clientAddress);

        SOCKET clientSocket = accept(
            listeningSocket,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressLength
        );

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection.\n";
            continue;
        }

        char clientIp[INET_ADDRSTRLEN]{};

        if (inet_ntop(AF_INET,
                      &clientAddress.sin_addr,
                      clientIp,
                      INET_ADDRSTRLEN) != nullptr) {
            std::cout << "Client connected from " << clientIp << '\n';
        }

        constexpr std::size_t maxMessageLength = 1024;
        char buffer[256];
        std::string message;
        bool completeMessageReceived = false;

        while (message.size() < maxMessageLength) {
            const int bytesReceived = recv(
                clientSocket,
                buffer,
                static_cast<int>(sizeof(buffer)),
                0
            );

            if (bytesReceived <= 0) {
                break;
            }

            message.append(buffer, static_cast<std::size_t>(bytesReceived));

            const std::size_t newlinePosition = message.find('\n');

            if (newlinePosition != std::string::npos) {
                message.resize(newlinePosition + 1);
                completeMessageReceived = true;
                break;
            }
        }

        if (!completeMessageReceived) {
            std::cerr << "Client sent an incomplete message.\n";
            closesocket(clientSocket);
            continue;
        }

        const std::string prefix = "DEVICE_ID=";

        if (message.rfind(prefix, 0) != 0) {
            std::cerr << "Invalid message format.\n";
            closesocket(clientSocket);
            continue;
        }

        std::string deviceId = message.substr(prefix.size());

        while (!deviceId.empty() &&
               (deviceId.back() == '\n' || deviceId.back() == '\r')) {
            deviceId.pop_back();
        }

        if (deviceId.empty()) {
            std::cerr << "Device ID cannot be empty.\n";
            closesocket(clientSocket);
            continue;
        }

        std::cout << "Device accepted: " << deviceId << '\n';

        closesocket(clientSocket);
    }
}



