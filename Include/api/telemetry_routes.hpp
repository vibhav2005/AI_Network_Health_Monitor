#pragma once

#include <crow.h>

#include "database/database_toolkit.hpp"

void registerTelemetryRoutes(crow::SimpleApp& app,
                            DatabaseToolkit& database);
