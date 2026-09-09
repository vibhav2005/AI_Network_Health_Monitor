#include "client/client_telemetry.hpp"
#include <random>
#include <chrono>

clientTelemetry::clientTelemetry(const std::string& deviceId, int cpuUsage, int memoryUsage,
        int gpuUsage, int latencyMs)
    : deviceId(deviceId), cpuUsage(cpuUsage), memoryUsage(memoryUsage),
      gpuUsage(gpuUsage), latencyMs(latencyMs) {}


void clientTelemetry::UpdateTelemetry(const std::string& newDeviceId, int cpuUsage, int memoryUsage,
        int gpuUsage, int latencyMs) {

    std::mt19937 rng(std::random_device{}());
    auto randomInRange = [&rng](int minValue, int maxValue) {
        std::uniform_int_distribution<int> dist(minValue, maxValue);
        return dist(rng);
    };

    deviceId = newDeviceId;
    this->cpuUsage = randomInRange(0, 100);
    this->memoryUsage = randomInRange(0, 100);
    this->gpuUsage = randomInRange(0, 100);
    this->latencyMs = randomInRange(30, 250);
}
    
int clientTelemetry::getCpuUsage() const {
    return cpuUsage;
}
int clientTelemetry::getMemoryUsage() const {
    return memoryUsage;
}
int clientTelemetry::getGpuUsage() const {
    return gpuUsage;
}
int clientTelemetry::getLatencyMs() const {
    return latencyMs;
}
const std::string& clientTelemetry::getDeviceId() const {
    return deviceId;
}

std::string clientTelemetry::toJson() const {
    return "{ \"deviceId\": \"" + deviceId + "\", \"cpuUsage\": " + std::to_string(cpuUsage) +
           ", \"memoryUsage\": " + std::to_string(memoryUsage) + ", \"gpuUsage\": " +
           std::to_string(gpuUsage) + ", \"latencyMs\": " + std::to_string(latencyMs) + " }";
}
