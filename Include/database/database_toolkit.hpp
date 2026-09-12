#ifndef DATABASE_TOOLKIT_HPP
#define DATABASE_TOOLKIT_HPP

#include <string>
#include <vector>

#include "database/database_manager.hpp"

struct DeviceRecord
{
    int id = 0;
    std::string name;
    std::string ipAddress;
    std::string deviceType;
    std::string status;
    std::string createdAt;
};

struct TelemetryRecord
{
    int id = 0;
    int deviceId = 0;
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    double temperature = 0.0;
    std::string timestamp;
};

struct AlertRecord
{
    int id = 0;
    int deviceId = 0;
    std::string severity;
    std::string message;
    std::string createdAt;
    bool resolved = false;
};

class DatabaseToolkit
{
public:
    DatabaseToolkit();
    explicit DatabaseToolkit(const database_config& config);

    bool connect(const database_config& config);
    bool isConnected() const;

    bool insertDevice(const std::string& name,
                      const std::string& ipAddress,
                      const std::string& deviceType,
                      const std::string& status = "online");
    bool addDevice(const std::string& name,
                   const std::string& ipAddress,
                   const std::string& deviceType,
                   const std::string& status = "online")
    {
        return insertDevice(name, ipAddress, deviceType, status);
    }

    bool insertTelemetry(int deviceId,
                         double cpuUsage,
                         double memoryUsage,
                         double temperature);
    bool addTelemetry(int deviceId,
                      double cpuUsage,
                      double memoryUsage,
                      double temperature)
    {
        return insertTelemetry(deviceId, cpuUsage, memoryUsage, temperature);
    }

    bool insertAlert(int deviceId,
                     const std::string& severity,
                     const std::string& message,
                     bool resolved = false);
    bool addAlert(int deviceId,
                  const std::string& severity,
                  const std::string& message,
                  bool resolved = false)
    {
        return insertAlert(deviceId, severity, message, resolved);
    }

    std::vector<DeviceRecord> viewDevices() const;
    std::vector<TelemetryRecord> viewTelemetryByDevice(int deviceId) const;
    std::vector<AlertRecord> viewAlertsByDevice(int deviceId) const;

    bool checkDeviceExists(int deviceId) const;
    bool checkTelemetryExists(int telemetryId) const;
    bool checkAlertExists(int alertId) const;

    std::string getServerAddress() const;
    std::string getServerVersion() const;

private:
    connection_manager connection_;
    query_execution executor_;
    server_information info_;
};

#endif // DATABASE_TOOLKIT_HPP
