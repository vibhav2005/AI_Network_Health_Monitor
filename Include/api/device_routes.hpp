#pragma once

#include <crow.h>

#include "database/database_toolkit.hpp"

void registerDeviceRoutes(crow::SimpleApp& app,
                         DatabaseToolkit& database);
