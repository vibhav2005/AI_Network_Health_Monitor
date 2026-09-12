#include "database/database_manager.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace
{
std::string trim(const std::string& input)
{
    const auto start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }

    const auto end = input.find_last_not_of(" \t\r\n");
    return input.substr(start, end - start + 1);
}
}

database_config::database_config()
    : host("localhost"),
      port(5432),
      dbname("postgres"),
      user("postgres"),
      password("")
{
}

database_manager::database_manager() = default;

database_manager::~database_manager() = default;

bool database_manager::loadFromEnvFile(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto equalPos = line.find('=');
        if (equalPos == std::string::npos)
        {
            continue;
        }

        std::string key = trim(line.substr(0, equalPos));
        std::string value = trim(line.substr(equalPos + 1));

        if (key.rfind("set ", 0) == 0)
        {
            key = trim(key.substr(4));
        }

        values[key] = value;
    }

    database_config cfg;
    cfg.host = values.count("DB_HOST") ? values["DB_HOST"] : cfg.host;
    cfg.port = values.count("DB_PORT") ? std::stoi(values["DB_PORT"]) : cfg.port;
    cfg.dbname = values.count("DB_NAME") ? values["DB_NAME"] : cfg.dbname;
    cfg.user = values.count("DB_USER") ? values["DB_USER"] : cfg.user;
    cfg.password = values.count("DB_PASSWORD") ? values["DB_PASSWORD"] : cfg.password;

    loadedConfig_ = cfg;
    return true;
}

bool database_manager::connect(const database_config& config)
{
    loadedConfig_ = config;
    if (!connection_.connect(config))
    {
        return false;
    }

    executor_ = query_execution(connection_.getConnectionHandle());
    info_ = server_information(connection_.getConnectionHandle());
    return true;
}

bool database_manager::executeQuery(const std::string& query)
{
    return executor_.executeQuery(query);
}

bool database_manager::executePrepared(const std::string& query,
                                        const std::vector<std::string>& params)
{
    return executor_.executePrepared(query, params);
}

std::string database_manager::getServerAddress() const
{
    return info_.getServerAddress();
}

std::string database_manager::getServerVersion() const
{
    return info_.getServerVersion();
}

std::string database_manager::getTransactionStatus() const
{
    return info_.getTransactionStatus();
}

database_config database_manager::getLoadedConfig() const
{
    return loadedConfig_;
}

connection_manager::connection_manager() = default;

connection_manager::~connection_manager()
{
    disconnect();
}

bool connection_manager::connect(const database_config& config)
{
    disconnect();
    config_ = config;

    std::ostringstream connInfo;
    connInfo << "host=" << config_.host
             << " port=" << config_.port
             << " dbname=" << config_.dbname
             << " user=" << config_.user
             << " password=" << config_.password;

    conn_ = PQconnectdb(connInfo.str().c_str());
    if (conn_ == nullptr)
    {
        return false;
    }

    if (PQstatus(conn_) != CONNECTION_OK)
    {
        std::string error = PQerrorMessage(conn_);
        (void)error;
        disconnect();
        return false;
    }

    return true;
}

void connection_manager::disconnect()
{
    if (conn_ != nullptr)
    {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

bool connection_manager::isConnected() const
{
    return conn_ != nullptr && PQstatus(conn_) == CONNECTION_OK;
}

database_config connection_manager::getConnectionConfig() const
{
    return config_;
}

bool connection_manager::getConnectionStatus(database_config& outConfig) const
{
    if (!isConnected())
    {
        return false;
    }

    outConfig = config_;
    return true;
}

bool connection_manager::getConnectionStatus(std::string& host,
                                             int& port,
                                             std::string& dbname,
                                             std::string& user,
                                             std::string& password) const
{
    if (!isConnected())
    {
        return false;
    }

    host = config_.host;
    port = config_.port;
    dbname = config_.dbname;
    user = config_.user;
    password = config_.password;
    return true;
}

PGconn* connection_manager::getConnectionHandle() const
{
    return conn_;
}

server_information::server_information()
    : conn_(nullptr),
      serverAddress_("localhost"),
      serverVersion_("unknown"),
      serverParameters_("not loaded")
{
}

server_information::server_information(PGconn* conn)
    : conn_(conn)
{
    if (conn_ != nullptr)
    {
        serverAddress_ = PQhost(conn_);
        if (serverAddress_.empty())
        {
            serverAddress_ = "localhost";
        }

        const char* port = PQport(conn_);
        if (port != nullptr && std::string(port) != "")
        {
            serverAddress_ += ":" + std::string(port);
        }

        const int version = PQserverVersion(conn_);
        if (version > 0)
        {
            const int major = version / 10000;
            const int minor = (version % 10000) / 100;
            const int patch = version % 100;
            std::ostringstream oss;
            oss << major << "." << minor;
            if (patch != 0)
            {
                oss << "." << patch;
            }
            serverVersion_ = "PostgreSQL " + oss.str();
        }

        serverParameters_ = PQparameterStatus(conn_, "server_version");
        if (serverParameters_.empty())
        {
            serverParameters_ = "unknown";
        }
    }
}

server_information::~server_information() = default;

void server_information::attachConnection(PGconn* conn)
{
    conn_ = conn;
    serverAddress_ = "localhost";
    serverVersion_ = "unknown";
    serverParameters_ = "not loaded";

    if (conn_ == nullptr)
    {
        return;
    }

    serverAddress_ = PQhost(conn_);
    if (serverAddress_.empty())
    {
        serverAddress_ = "localhost";
    }

    const char* port = PQport(conn_);
    if (port != nullptr && std::string(port) != "")
    {
        serverAddress_ += ":" + std::string(port);
    }

    const int version = PQserverVersion(conn_);
    if (version > 0)
    {
        const int major = version / 10000;
        const int minor = (version % 10000) / 100;
        const int patch = version % 100;
        std::ostringstream oss;
        oss << major << "." << minor;
        if (patch != 0)
        {
            oss << "." << patch;
        }
        serverVersion_ = "PostgreSQL " + oss.str();
    }

    const char* serverVersionParam = PQparameterStatus(conn_, "server_version");
    if (serverVersionParam != nullptr)
    {
        serverParameters_ = serverVersionParam;
    }
}

std::string server_information::getServerAddress() const
{
    return serverAddress_;
}

std::string server_information::getServerVersion() const
{
    return serverVersion_;
}

std::string server_information::getServerParameters() const
{
    return serverParameters_;
}

std::string server_information::getTransactionStatus() const
{
    if (conn_ == nullptr)
    {
        return "disconnected";
    }

    switch (PQtransactionStatus(conn_))
    {
        case PQTRANS_IDLE:
            return "idle";
        case PQTRANS_ACTIVE:
            return "active";
        case PQTRANS_INTRANS:
            return "in transaction";
        case PQTRANS_INERROR:
            return "error";
        case PQTRANS_UNKNOWN:
            return "unknown";
        default:
            return "unknown";
    }
}

query_execution::query_execution(PGconn* conn)
    : conn_(conn), lastStatus_(PGRES_EMPTY_QUERY)
{
}

query_execution::~query_execution() = default;

bool query_execution::executeQuery(const std::string& query)
{
    if (conn_ == nullptr)
    {
        return false;
    }

    PGresult* result = PQexec(conn_, query.c_str());
    if (result == nullptr)
    {
        lastStatus_ = PGRES_FATAL_ERROR;
        return false;
    }

    lastStatus_ = PQresultStatus(result);
    bool ok = (lastStatus_ == PGRES_COMMAND_OK || lastStatus_ == PGRES_TUPLES_OK);
    PQclear(result);
    return ok;
}

bool query_execution::executePrepared(const std::string& query,
                                     const std::vector<std::string>& params)
{
    if (conn_ == nullptr)
    {
        return false;
    }

    std::vector<const char*> values;
    values.reserve(params.size());
    for (const auto& param : params)
    {
        values.push_back(param.c_str());
    }

    PGresult* result = PQexecParams(conn_,
                                   query.c_str(),
                                   static_cast<int>(params.size()),
                                   nullptr,
                                   values.empty() ? nullptr : values.data(),
                                   nullptr,
                                   nullptr,
                                   0);

    if (result == nullptr)
    {
        lastStatus_ = PGRES_FATAL_ERROR;
        return false;
    }

    lastStatus_ = PQresultStatus(result);
    bool ok = (lastStatus_ == PGRES_COMMAND_OK || lastStatus_ == PGRES_TUPLES_OK);
    PQclear(result);
    return ok;
}

bool query_execution::beginTransaction()
{
    return executeQuery("BEGIN;");
}

bool query_execution::commit()
{
    return executeQuery("COMMIT;");
}

bool query_execution::rollback()
{
    return executeQuery("ROLLBACK;");
}

bool query_execution::checkQueryResult() const
{
    return lastStatus_ == PGRES_COMMAND_OK || lastStatus_ == PGRES_TUPLES_OK;
}
