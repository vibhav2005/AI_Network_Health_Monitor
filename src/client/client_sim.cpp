#include "client/client_sim.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>


ClientSim::ClientSim(const std::string& serverAddress,
                     std::uint16_t serverPort,
                     const std::string& deviceId)
    : serverAddr(serverAddress),
      serverPort(serverPort),
      deviceId(deviceId),
      telemetry(deviceId, 0, 0, 0, 0),
      connected(false),
      clientSocket(INVALID_SOCKET),
      telemetryThread() {
    telemetryThread = std::thread([this]() {
        sendTelemetryStream(500);
    });
}

ClientSim::~ClientSim() {
    connected = false;

    if (telemetryThread.joinable()) {
        telemetryThread.join();
    }

    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
}

bool ClientSim::connectToServer() {
    if (deviceId.empty()) {
        std::cerr << "Device ID cannot be empty.\n";
        return false;
    }

    if (clientSocket != INVALID_SOCKET) {
        return true;
    }

    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
        clientSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    if (connect(clientSocket,
                reinterpret_cast<sockaddr*>(&serverAddress),
                sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to "
                  << serverAddr << ':' << serverPort << '\n';
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }

    connected = true;
    std::cout << "Connected to server: " << serverAddr << ':' << serverPort << '\n';
    return true;
}

bool ClientSim::generateTelemetry() {
    if (deviceId.empty()) {
        std::cerr << "Device ID cannot be empty.\n";
        return false;
    }

    telemetry.UpdateTelemetry(deviceId, 0, 0, 0, 0);
    return true;
}

std::string ClientSim::serializeTelemetryJson() const {
    return telemetry.toJson();
}

bool ClientSim::sendData(const std::string& payload) {
    if (!connected || clientSocket == INVALID_SOCKET) {
        return false;
    }

    const std::string message = payload + "\n";
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
            std::cerr << "Failed to send telemetry payload.\n";
            connected = false;
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            WSACleanup();
            return false;
        }

        totalBytesSent += bytesSent;
    }

    std::cout << "Sent telemetry JSON for device: " << deviceId << '\n';
    return true;
}

bool ClientSim::sendTelemetry() {
    if (!generateTelemetry()) {
        return false;
    }

    return sendData(serializeTelemetryJson());
}

void ClientSim::sendTelemetryStream(int delayMs) {
    if (!connectToServer()) {
        return;
    }

    while (connected) {
        if (!generateTelemetry()) {
            break;
        }

        std::string payload = serializeTelemetryJson();
        if (!sendData(payload)) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }

    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }

    connected = false;
    WSACleanup();
}

bool ClientSim::connectAndSend() {
    if (!connectToServer()) {
        return false;
    }

    while (connected) {
        if (!generateTelemetry()) {
            break;
        }

        if (!sendData(serializeTelemetryJson())) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return connected;
}

bool ClientSim::isConnected() const {
    return connected;
}