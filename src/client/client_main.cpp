#include "client/client_sim.hpp"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    ClientSim client("127.0.0.1", 9000, "device-001");
    ClientSim client2("127.0.0.1", 9000, "device-002");

    std::cout << "Both client telemetry streams started.\n";

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}