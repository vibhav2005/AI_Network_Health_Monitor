#ifndef CLIENT_TELEMETRY
#define CLIENT_TELEMETRY
#include <string>
#include <iostream>
class clientTelemetry{
    private:
        std::string deviceId;
        int cpuUsage;
        int memoryUsage;
        int gpuUsage;
        int latencyMs;
    public:
    // Constructor
        clientTelemetry(const std::string& deviceId, int cpuUsage, int memoryUsage, 
        int gpuUsage, int latencyMs);
    // Update telemetry data for object 
        void UpdateTelemetry(const std::string& newDeviceId, int cpuUsage, int memoryUsage,
        int gpuUsage, int latencyMs);
    // convert telemetry data to JSON format
        std::string toJson() const;
    //getters for telemetry data
        const std::string& getDeviceId() const;
        int getCpuUsage() const;
        int getMemoryUsage() const;
        int getGpuUsage() const;
        int getLatencyMs() const;

};
 
#endif//client telemetry hpp