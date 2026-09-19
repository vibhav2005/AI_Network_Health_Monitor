#pragma once

#include <crow.h>

#include "database/database_toolkit.hpp"

void registerAlertRoutes(crow::SimpleApp& app,
                        DatabaseToolkit& database);
