#ifndef UNITY_SERVER_DBUTILS_H
#define UNITY_SERVER_DBUTILS_H
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <hiredis/hiredis.h>
#include <unordered_set>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

struct databaseManager{
    sql::mysql::MySQL_Driver *driver{};
    sql::Connection *con{};
    sql::Statement *stmt{};
    redisContext *context{};
    std::unordered_set<std::string> loged_users;
    std::mt19937 mt{};

    int rng(int l, int r);

    databaseManager() = default;

    ~databaseManager();

    bool login(const std::string& username, const std::string& password);

    static std::string getCurrentTimestamp();

    void logout(const std::string& username);

    void setKey(const std::string& username, const std::string& password);

    int getKey(const std::string& username, std::string& key) const;

    void init_link();

    void init_data();
};
#endif //UNITY_SERVER_DBUTILS_H
