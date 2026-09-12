#include "database/database_toolkit.hpp"

#include <sstream>
#include <string>

namespace
{
std::string escapeSqlLiteral(const std::string& value)
{
    std::string result;
    result.reserve(value.size());
    for (char ch : value)
    {
        if (ch == '\'')
        {
            result += "''";
        }
        else
        {
            result += ch;
        }
    }
    return result;
}

std::string formatBool(bool value)
{
    return value ? "true" : "false";
}
}

DatabaseToolkit::DatabaseToolkit() = default;

DatabaseToolkit::DatabaseToolkit(const database_config& config)
{
    connect(config);
}

bool DatabaseToolkit::connect(const database_config& config)
{
    if (!connection_.connect(config))
    {
        return false;
    }

    executor_ = query_execution(connection_.getConnectionHandle());
    info_ = server_information(connection_.getConnectionHandle());
    return true;
}

bool DatabaseToolkit::isConnected() const
{
    return connection_.isConnected();
}

bool DatabaseToolkit::insertDevice(const std::string& name,
                                  const std::string& ipAddress,
                                  const std::string& deviceType,
                                  const std::string& status)
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "INSERT INTO devices (name, ip_address, device_type, status, created_at) VALUES ("
        << "'" << escapeSqlLiteral(name) << "', "
        << "'" << escapeSqlLiteral(ipAddress) << "', "
        << "'" << escapeSqlLiteral(deviceType) << "', "
        << "'" << escapeSqlLiteral(status) << "', "
        << "NOW()" << ");";

    return executor_.executeQuery(sql.str());
}

bool DatabaseToolkit::insertTelemetry(int deviceId,
                                     double cpuUsage,
                                     double memoryUsage,
                                     double temperature)
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "INSERT INTO telemetry (device_id, cpu_usage, memory_usage, temperature, timestamp) VALUES ("
        << deviceId << ", "
        << cpuUsage << ", "
        << memoryUsage << ", "
        << temperature << ", "
        << "NOW());";

    return executor_.executeQuery(sql.str());
}

bool DatabaseToolkit::insertAlert(int deviceId,
                                 const std::string& severity,
                                 const std::string& message,
                                 bool resolved)
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "INSERT INTO alerts (device_id, severity, message, created_at, resolved) VALUES ("
        << deviceId << ", "
        << "'" << escapeSqlLiteral(severity) << "', "
        << "'" << escapeSqlLiteral(message) << "', "
        << "NOW(), "
        << formatBool(resolved) << ");";

    return executor_.executeQuery(sql.str());
}

std::vector<DeviceRecord> DatabaseToolkit::viewDevices() const
{
    std::vector<DeviceRecord> records;
    if (!connection_.isConnected())
    {
        return records;
    }

    PGresult* result = PQexec(connection_.getConnectionHandle(),
                             "SELECT id, name, ip_address, device_type, status, created_at FROM devices ORDER BY id;");
    if (result == nullptr || PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        if (result != nullptr)
        {
            PQclear(result);
        }
        return records;
    }

    const int rowCount = PQntuples(result);
    for (int i = 0; i < rowCount; ++i)
    {
        DeviceRecord record;
        record.id = std::stoi(PQgetvalue(result, i, 0));
        record.name = PQgetvalue(result, i, 1);
        record.ipAddress = PQgetvalue(result, i, 2);
        record.deviceType = PQgetvalue(result, i, 3);
        record.status = PQgetvalue(result, i, 4);
        record.createdAt = PQgetvalue(result, i, 5);
        records.push_back(record);
    }

    PQclear(result);
    return records;
}

std::vector<TelemetryRecord> DatabaseToolkit::viewTelemetryByDevice(int deviceId) const
{
    std::vector<TelemetryRecord> records;
    if (!connection_.isConnected())
    {
        return records;
    }

    std::ostringstream sql;
    sql << "SELECT id, device_id, cpu_usage, memory_usage, temperature, timestamp FROM telemetry WHERE device_id = "
        << deviceId << " ORDER BY id;";

    PGresult* result = PQexec(connection_.getConnectionHandle(), sql.str().c_str());
    if (result == nullptr || PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        if (result != nullptr)
        {
            PQclear(result);
        }
        return records;
    }

    const int rowCount = PQntuples(result);
    for (int i = 0; i < rowCount; ++i)
    {
        TelemetryRecord record;
        record.id = std::stoi(PQgetvalue(result, i, 0));
        record.deviceId = std::stoi(PQgetvalue(result, i, 1));
        record.cpuUsage = std::stod(PQgetvalue(result, i, 2));
        record.memoryUsage = std::stod(PQgetvalue(result, i, 3));
        record.temperature = std::stod(PQgetvalue(result, i, 4));
        record.timestamp = PQgetvalue(result, i, 5);
        records.push_back(record);
    }

    PQclear(result);
    return records;
}

std::vector<AlertRecord> DatabaseToolkit::viewAlertsByDevice(int deviceId) const
{
    std::vector<AlertRecord> records;
    if (!connection_.isConnected())
    {
        return records;
    }

    std::ostringstream sql;
    sql << "SELECT id, device_id, severity, message, created_at, resolved FROM alerts WHERE device_id = "
        << deviceId << " ORDER BY id;";

    PGresult* result = PQexec(connection_.getConnectionHandle(), sql.str().c_str());
    if (result == nullptr || PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        if (result != nullptr)
        {
            PQclear(result);
        }
        return records;
    }

    const int rowCount = PQntuples(result);
    for (int i = 0; i < rowCount; ++i)
    {
        AlertRecord record;
        record.id = std::stoi(PQgetvalue(result, i, 0));
        record.deviceId = std::stoi(PQgetvalue(result, i, 1));
        record.severity = PQgetvalue(result, i, 2);
        record.message = PQgetvalue(result, i, 3);
        record.createdAt = PQgetvalue(result, i, 4);
        record.resolved = std::string(PQgetvalue(result, i, 5)) == "t";
        records.push_back(record);
    }

    PQclear(result);
    return records;
}

bool DatabaseToolkit::checkDeviceExists(int deviceId) const
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "SELECT 1 FROM devices WHERE id = " << deviceId << " LIMIT 1;";

    PGresult* result = PQexec(connection_.getConnectionHandle(), sql.str().c_str());
    if (result == nullptr)
    {
        return false;
    }

    const bool exists = (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0);
    PQclear(result);
    return exists;
}

bool DatabaseToolkit::checkTelemetryExists(int telemetryId) const
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "SELECT 1 FROM telemetry WHERE id = " << telemetryId << " LIMIT 1;";

    PGresult* result = PQexec(connection_.getConnectionHandle(), sql.str().c_str());
    if (result == nullptr)
    {
        return false;
    }

    const bool exists = (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0);
    PQclear(result);
    return exists;
}

bool DatabaseToolkit::checkAlertExists(int alertId) const
{
    if (!connection_.isConnected())
    {
        return false;
    }

    std::ostringstream sql;
    sql << "SELECT 1 FROM alerts WHERE id = " << alertId << " LIMIT 1;";

    PGresult* result = PQexec(connection_.getConnectionHandle(), sql.str().c_str());
    if (result == nullptr)
    {
        return false;
    }

    const bool exists = (PQresultStatus(result) == PGRES_TUPLES_OK && PQntuples(result) > 0);
    PQclear(result);
    return exists;
}

std::string DatabaseToolkit::getServerAddress() const
{
    return info_.getServerAddress();
}

std::string DatabaseToolkit::getServerVersion() const
{
    return info_.getServerVersion();
}
