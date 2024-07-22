#ifndef MYSQL_H
#define MYSQL_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#include <iostream>
#include <string>
using namespace std;

class MySQL
{
private:
    const string db_host = "tcp://127.0.0.1:3306";
    const string db_user;
    const string db_pass;
    const string db_name;
    sql::Connection* con;

public:
    MySQL(string db_user, string db_pass, string db_name);
    ~MySQL();
    void createDatabase(const std::string& database_name);
    bool tableExists(const std::string& table_name);
    void createTable(const std::string& table_name);
    bool insertUser(const std::string& username, const std::string& password);
    string queryUserPassword(const std::string& username);
};
#endif