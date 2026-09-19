#pragma once

#include <crow.h>

#include "database/database_toolkit.hpp"

void registerSystemRoutes(crow::SimpleApp& app,
                          const DatabaseToolkit& database);