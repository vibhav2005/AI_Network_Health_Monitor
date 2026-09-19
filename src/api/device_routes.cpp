#include "api/device_routes.hpp"
#include <string>
namespace
{
crow::response jsonResponse(int statusCode, crow::json::wvalue body)
{
    return crow::response(statusCode, std::move(body));
}

crow::json::wvalue makeDeviceJson(const DeviceRecord& device)
{
    crow::json::wvalue item;
    item["id"] = device.id;
    item["name"] = device.name;
    item["ip_address"] = device.ipAddress;
    item["device_type"] = device.deviceType;
    item["status"] = device.status;
    item["created_at"] = device.createdAt;
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

void registerDeviceRoutes(crow::SimpleApp& app,
                         DatabaseToolkit& database)
{
    CROW_ROUTE(app, "/api/devices")([&database](const crow::request& req)
    {
        if (req.method == crow::HTTPMethod::GET)
        {
            const auto devices = database.viewDevices();
            crow::json::wvalue response;
            crow::json::wvalue::list list;

            for (const auto& device : devices)
            {
                list.push_back(makeDeviceJson(device));
            }

            response["devices"] = std::move(list);
            response["count"] = static_cast<int>(devices.size());
            return jsonResponse(200, std::move(response));
        }

        if (req.method == crow::HTTPMethod::POST)
        {
            auto body = crow::json::load(req.body);
            if (!body)
            {
                return jsonResponse(400, makeErrorJson("Invalid JSON body", 400));
            }

            std::string name;
            std::string ipAddress;
            std::string deviceType;
            std::string status = "online";

            if (!readStringField(body, "name", name) ||
                !readStringField(body, "ip_address", ipAddress) ||
                !readStringField(body, "device_type", deviceType))
            {
                return jsonResponse(400, makeErrorJson("name, ip_address, and device_type are required", 400));
            }

            if (body.has("status") && body["status"].t() == crow::json::type::String)
            {
                status = body["status"].s();
            }

            if (!database.insertDevice(name, ipAddress, deviceType, status))
            {
                return jsonResponse(500, makeErrorJson("Failed to create device", 500));
            }

            crow::json::wvalue response;
            response["message"] = "Device created";
            response["status"] = "success";
            return jsonResponse(201, std::move(response));
        }

        return jsonResponse(405, makeErrorJson("Method not allowed", 405));
    });

    CROW_ROUTE(app, "/api/devices/<int>")([&database](int deviceId)
    {
        if (!database.checkDeviceExists(deviceId))
        {
            return jsonResponse(404, makeErrorJson("Device not found", 404));
        }

        const auto devices = database.viewDevices();
        for (const auto& device : devices)
        {
            if (device.id == deviceId)
            {
                return jsonResponse(200, makeDeviceJson(device));
            }
        }

        return jsonResponse(404, makeErrorJson("Device not found", 404));
    });
}
