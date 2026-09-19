#include "api/system_routes.hpp"

namespace
{
crow::response jsonResponse(int statusCode, crow::json::wvalue body)
{
    return crow::response(statusCode, std::move(body));
}
}

void registerSystemRoutes(crow::SimpleApp& app,
                          const DatabaseToolkit& database)
{
    CROW_ROUTE(app, "/health")([]
    {
        crow::json::wvalue response;
        response["status"] = "ok";

        return jsonResponse(200, std::move(response));
    });

    CROW_ROUTE(app, "/api/status")([&database]
    {
        crow::json::wvalue response;
        response["service"] = "network-health-monitor";
        response["status"] = "running";
        response["database_connected"] = database.isConnected();

        if (database.isConnected())
        {
            response["database_server"] = database.getServerAddress();
            response["database_version"] = database.getServerVersion();
        }

        return jsonResponse(200, std::move(response));
    });
}