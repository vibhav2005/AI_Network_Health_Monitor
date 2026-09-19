#include "api/rest_server.hpp"

#include "api/system_routes.hpp"
#include "database/database_manager.hpp"

#include <iostream>

void RestServer::run(unsigned short port)
{
	crow::SimpleApp app;

	database_manager config_loader;
	config_loader.loadFromEnvFile();
	database_.connect(config_loader.getLoadedConfig());

	registerSystemRoutes(app, database_);

	std::cout << "REST server listening on http://localhost:" << port << '\n';
	app.port(port).multithreaded().run();
}
