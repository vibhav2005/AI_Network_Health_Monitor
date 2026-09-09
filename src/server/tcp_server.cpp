#include "server/tcp_server.hpp"

#include "winsock2.h"
#include "ws2tcpip.h"

#include <iostream>
#include <thread>

TcpServer::TcpServer(const std::string& address, std::uint16_t port):
    address(address),
    port(port) {}

bool TcpServer::acceptClient(SOCKET listeningSocket,
                            SOCKET& clientSocket,
                            std::string& clientIp) {
    if (listeningSocket == INVALID_SOCKET) {
        std::cerr << "Invalid listening socket.\n";
        return false;
    }

    sockaddr_in clientAddress{};
    int clientAddressLength = sizeof(clientAddress);

    clientSocket = accept(listeningSocket,
                          reinterpret_cast<sockaddr*>(&clientAddress),
                          &clientAddressLength);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection.\n";
        return false;
    }

    char ipBuffer[INET_ADDRSTRLEN]{};
    if (inet_ntop(AF_INET,
                  &clientAddress.sin_addr,
                  ipBuffer,
                  INET_ADDRSTRLEN) != nullptr) {
        clientIp = ipBuffer;
    } else {
        clientIp = "unknown";
    }

    return true;
}

bool TcpServer::receiveMessage(SOCKET clientSocket, std::string& message) {
    constexpr std::size_t maxMessageLength = 1024;
    char buffer[256]{};
    message.clear();

    while (message.size() < maxMessageLength) {
        const int bytesReceived = recv(
            clientSocket,
            buffer,
            static_cast<int>(sizeof(buffer)),
            0
        );

        if (bytesReceived <= 0) {
            return false;
        }

        message.append(buffer, static_cast<std::size_t>(bytesReceived));

        const std::size_t newlinePosition = message.find('\n');

        if (newlinePosition != std::string::npos) {
            message.resize(newlinePosition + 1);
            return true;
        }
    }

    std::cerr << "Client message exceeded maximum length.\n";
    return false;
}

bool TcpServer::parseMessage(const std::string& message, std::string& deviceId) {
    if (message.empty()) {
        return false;
    }

    const std::string prefix = "DEVICE_ID=";
    if (message.rfind(prefix, 0) == 0) {
        deviceId = message.substr(prefix.size());
        while (!deviceId.empty() && (deviceId.back() == '\n' || deviceId.back() == '\r')) {
            deviceId.pop_back();
        }
        return !deviceId.empty();
    }

    const std::size_t deviceIdPos = message.find("\"deviceId\"");
    if (deviceIdPos == std::string::npos) {
        return false;
    }

    const std::size_t valueStart = message.find(':', deviceIdPos);
    const std::size_t valueFirstQuote = message.find('"', valueStart + 1);
    const std::size_t valueLastQuote = message.find('"', valueFirstQuote + 1);

    if (valueFirstQuote == std::string::npos || valueLastQuote == std::string::npos) {
        return false;
    }

    deviceId = message.substr(valueFirstQuote + 1, valueLastQuote - valueFirstQuote - 1);
    return !deviceId.empty();
}

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

    std::cout << "Server listening on " << address << ':' << port << '\n';

    while (true) {
        std::cout << "Waiting for a client connection...\n";

        SOCKET clientSocket = INVALID_SOCKET;
        std::string clientIp;
        if (!acceptClient(listeningSocket, clientSocket, clientIp)) {
            continue;
        }

        std::cout << "Client connected from " << clientIp << '\n';

        std::thread clientThread([this, clientSocket]() {
            while (true) {
                std::string message;
                if (!receiveMessage(clientSocket, message)) {
                    std::cerr << "Client disconnected.\n";
                    closesocket(clientSocket);
                    return;
                }

                std::string deviceId;
                if (!parseMessage(message, deviceId)) {
                    std::cerr << "Invalid message format.\n";
                    std::cerr << "Raw message: " << message << '\n';
                    continue;
                }

                std::cout << "Device accepted: " << deviceId << '\n';
                std::cout << "Received payload: " << message << '\n';
            }
        });

        clientThread.detach();
    }

    closesocket(listeningSocket);
    WSACleanup();
    return true;
}

