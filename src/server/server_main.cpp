#include "database/database_manager.hpp"
#include <iostream>

int main()
{
    database_manager db;

    if (!db.loadFromEnvFile("E:/CPP_Projects/AI_network_monitoring/.env"))
    {
        std::cout << "No .env file found. Using default config.\n";
    }

    database_config cfg = db.getLoadedConfig();
    std::cout << "Loaded host=" << cfg.host
              << " port=" << cfg.port
              << " db=" << cfg.dbname
              << " user=" << cfg.user << "\n";

    if (!db.connect(cfg))
    {
        std::cerr << "Database connection failed.\n";
        return 1;
    }

    std::cout << "Connected to PostgreSQL\n";
    std::cout << "Server address: " << db.getServerAddress() << "\n";
    std::cout << "Server version: " << db.getServerVersion() << "\n";
    std::cout << "Transaction status: " << db.getTransactionStatus() << "\n";

    db.executeQuery("SELECT * FROM devices;");

    return 0;
}