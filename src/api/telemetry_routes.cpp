#include "api/telemetry_routes.hpp"

namespace
{
crow::response jsonResponse(int statusCode, crow::json::wvalue body)
{
    return crow::response(statusCode, std::move(body));
}

crow::json::wvalue makeTelemetryJson(const TelemetryRecord& telemetry)
{
    crow::json::wvalue item;
    item["id"] = telemetry.id;
    item["device_id"] = telemetry.deviceId;
    item["cpu_usage"] = telemetry.cpuUsage;
    item["memory_usage"] = telemetry.memoryUsage;
    item["temperature"] = telemetry.temperature;
    item["timestamp"] = telemetry.timestamp;
    return item;
}

crow::json::wvalue makeErrorJson(const std::string& message, int statusCode)
{
    crow::json::wvalue body;
    body["error"] = message;
    body["status"] = statusCode;
    return body;
}
}

void registerTelemetryRoutes(crow::SimpleApp& app,
                            DatabaseToolkit& database)
{
    CROW_ROUTE(app, "/api/devices/<int>/telemetry")([&database](int deviceId)
    {
        if (!database.checkDeviceExists(deviceId))
        {
            return jsonResponse(404, makeErrorJson("Device not found", 404));
        }

        const auto telemetryRecords = database.viewTelemetryByDevice(deviceId);
        crow::json::wvalue response;
        crow::json::wvalue::list list;

        for (const auto& telemetry : telemetryRecords)
        {
            list.push_back(makeTelemetryJson(telemetry));
        }

        response["device_id"] = deviceId;
        response["telemetry"] = std::move(list);
        response["count"] = static_cast<int>(telemetryRecords.size());
        return jsonResponse(200, std::move(response));
    });

    CROW_ROUTE(app, "/api/devices/<int>/telemetry").methods(crow::HTTPMethod::POST)([&database](const crow::request& req, int deviceId)
    {
        if (!database.checkDeviceExists(deviceId))
        {
            return jsonResponse(404, makeErrorJson("Device not found", 404));
        }

        auto body = crow::json::load(req.body);
        if (!body)
        {
            return jsonResponse(400, makeErrorJson("Invalid JSON body", 400));
        }

        if (!body.has("cpu_usage") || !body.has("memory_usage") || !body.has("temperature"))
        {
            return jsonResponse(400, makeErrorJson("cpu_usage, memory_usage and temperature are required", 400));
        }

        const double cpuUsage = body["cpu_usage"].d();
        const double memoryUsage = body["memory_usage"].d();
        const double temperature = body["temperature"].d();

        if (!database.insertTelemetry(deviceId, cpuUsage, memoryUsage, temperature))
        {
            return jsonResponse(500, makeErrorJson("Failed to add telemetry", 500));
        }

        crow::json::wvalue response;
        response["message"] = "Telemetry added";
        response["device_id"] = deviceId;
        return jsonResponse(201, std::move(response));
    });
}
