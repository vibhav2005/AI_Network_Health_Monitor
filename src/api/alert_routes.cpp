#include "api/alert_routes.hpp"

namespace
{
crow::response jsonResponse(int statusCode, crow::json::wvalue body)
{
    return crow::response(statusCode, std::move(body));
}

crow::json::wvalue makeAlertJson(const AlertRecord& alert)
{
    crow::json::wvalue item;
    item["id"] = alert.id;
    item["device_id"] = alert.deviceId;
    item["severity"] = alert.severity;
    item["message"] = alert.message;
    item["created_at"] = alert.createdAt;
    item["resolved"] = alert.resolved;
    return item;
}

crow::json::wvalue makeErrorJson(const std::string& message, int statusCode)
{
    crow::json::wvalue body;
    body["error"] = message;
    body["status"] = statusCode;
    return body;
}

bool readStringField(const crow::json::rvalue& body, const char* key, std::string& output)
{
    if (!body.has(key) || body[key].t() != crow::json::type::String)
    {
        return false;
    }

    output = body[key].s();
    return true;
}
}

void registerAlertRoutes(crow::SimpleApp& app,
                        DatabaseToolkit& database)
{
    CROW_ROUTE(app, "/api/devices/<int>/alerts")([&database](int deviceId)
    {
        if (!database.checkDeviceExists(deviceId))
        {
            return jsonResponse(404, makeErrorJson("Device not found", 404));
        }

        const auto alertRecords = database.viewAlertsByDevice(deviceId);
        crow::json::wvalue response;
        crow::json::wvalue::list list;

        for (const auto& alert : alertRecords)
        {
            list.push_back(makeAlertJson(alert));
        }

        response["device_id"] = deviceId;
        response["alerts"] = std::move(list);
        response["count"] = static_cast<int>(alertRecords.size());
        return jsonResponse(200, std::move(response));
    });

    CROW_ROUTE(app, "/api/devices/<int>/alerts").methods(crow::HTTPMethod::POST)([&database](const crow::request& req, int deviceId)
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

        std::string severity;
        std::string message;
        bool resolved = false;

        if (!readStringField(body, "severity", severity) ||
            !readStringField(body, "message", message))
        {
            return jsonResponse(400, makeErrorJson("severity and message are required", 400));
        }

        if (body.has("resolved") && (body["resolved"].t() == crow::json::type::True || body["resolved"].t() == crow::json::type::False))
        {
            resolved = body["resolved"].b();
        }

        if (!database.insertAlert(deviceId, severity, message, resolved))
        {
            return jsonResponse(500, makeErrorJson("Failed to add alert", 500));
        }

        crow::json::wvalue response;
        response["message"] = "Alert added";
        response["device_id"] = deviceId;
        return jsonResponse(201, std::move(response));
    });
}
