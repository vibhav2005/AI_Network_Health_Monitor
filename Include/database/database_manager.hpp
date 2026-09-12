#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <iostream>
#include <string>
#include <vector>

#include "D:/postgreSql/include/libpq-fe.h"

struct database_config
{
    database_config();

    std::string host ;
    int port ;
    std::string dbname;
    std::string user;
    std::string password;
};

class connection_manager
{
public:
    connection_manager();
    ~connection_manager();

    bool connect(const database_config& config);
    void disconnect();
    bool isConnected() const;

    database_config getConnectionConfig() const;
    bool getConnectionStatus(database_config& outConfig) const;
    bool getConnectionStatus(std::string& host,
                             int& port,
                             std::string& dbname,
                             std::string& user,
                             std::string& password) const;
    PGconn* getConnectionHandle() const;

private:
    PGconn* conn_ = nullptr;
    database_config config_;
};

class server_information
{
public:
    server_information();
    explicit server_information(PGconn* conn);
    ~server_information();

    void attachConnection(PGconn* conn);

    std::string getServerAddress() const;
    std::string getServerVersion() const;
    std::string getServerParameters() const;
    std::string getTransactionStatus() const;

private:
    PGconn* conn_ = nullptr;
    std::string serverAddress_;
    std::string serverVersion_;
    std::string serverParameters_;
};

class query_execution
{
public:
    explicit query_execution(PGconn* conn = nullptr);
    ~query_execution();

    bool executeQuery(const std::string& query);
    bool executePrepared(const std::string& query,
                         const std::vector<std::string>& params);
    bool beginTransaction();
    bool commit();
    bool rollback();
    bool checkQueryResult() const;

private:
    PGconn* conn_ = nullptr;
    ExecStatusType lastStatus_ = PGRES_EMPTY_QUERY;
};

class database_manager
{
public:
    database_manager();
    ~database_manager();

    bool loadFromEnvFile(const std::string& filePath = ".env");
    bool connect(const database_config& config);
    bool executeQuery(const std::string& query);
    bool executePrepared(const std::string& query,
                         const std::vector<std::string>& params);

    std::string getServerAddress() const;
    std::string getServerVersion() const;
    std::string getTransactionStatus() const;
    database_config getLoadedConfig() const;

private:
    connection_manager connection_;
    query_execution executor_;
    server_information info_;
    database_config loadedConfig_;
};

#endif // DATABASE_MANAGER_HPP