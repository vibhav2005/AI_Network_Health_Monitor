#include "client/client_sim.hpp"
#include <iostream>

int main() {
    ClientSim client("127.0.0.1", 9000, "device-001");
    ClientSim client2("127.0.0.1", 9000, "device-002");

    if(client.connectAndSend()) {
        std::cout << "Client 1 connected and sent device ID successfully.\n";
    } else {
        std::cerr << "Client 1 failed to connect or send device ID.\n";
    }

    if(client2.connectAndSend()) {
        std::cout << "Client 2 connected and sent device ID successfully.\n";
    } else {
        std::cerr << "Client 2 failed to connect or send device ID.\n";
    }

    return 0;
}