#include "MySQL.h"

MySQL::MySQL(string db_user, string db_pass, string db_name):
db_user(db_user),db_pass(db_pass),db_name(db_name)
{
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    try{
        con = driver->connect(db_host, db_user, db_pass);
    }catch (sql::SQLException& e) {
        std::cerr << "MySQL connection failed!" << db_name << std::endl;
    }
    try {
        con->setSchema(db_name);
    } catch (sql::SQLException& e) {
        std::cerr << "Database does not exist, creating database: " << db_name << std::endl;
        createDatabase(db_name);
        con->setSchema(db_name);
    }
}

MySQL::~MySQL()
{

}

void MySQL::createDatabase(const std::string& database_name){
    try {
        sql::Statement* stmt = con->createStatement();
        stmt->execute("CREATE DATABASE IF NOT EXISTS " + database_name);
        std::cout << "Database created or already exists: " << database_name << std::endl;
        delete stmt;
    } catch (sql::SQLException& e) {
        std::cerr << "Error creating database: " << e.what() << std::endl;
    }
}

// bool MySQL::tableExists(const std::string& table_name) {
//     try {
//         sql::PreparedStatement* pstmt = con->prepareStatement("SHOW TABLES LIKE '"+ table_name + "'");
//         sql::ResultSet* res = pstmt->executeQuery();
//         bool exists = res->next();
//         delete res;
//         delete pstmt;
//         return exists;
//     } catch (sql::SQLException& e) {
//         std::cerr << "Error checking table existence: " << e.what() << std::endl;
//         return false;
//     }
// }

void MySQL::createTable(const std::string& table_name) {
    try {
        sql::Statement* stmt = con->createStatement();
        stmt->execute("CREATE TABLE IF NOT EXISTS " + table_name + "("
                      "id INT PRIMARY KEY AUTO_INCREMENT, "
                      "username VARCHAR(255) NOT NULL UNIQUE, "
                      "password VARCHAR(255) NOT NULL)");
        std::cout << "User table created successfully." << std::endl;
        delete stmt;
    } catch (sql::SQLException& e) {
        std::cerr << "Error creating table: " << e.what() << std::endl;
    }
}

bool MySQL::insertUser(const std::string& username, const std::string& password) {
    // if (!tableExists("user_table")) {
    //     std::cerr << "user_table does not exist, creating table: user_table" << std::endl;
    //     createTable("user_table");
    // }
    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("INSERT INTO user_table (username, password) VALUES (?, ?)");
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        pstmt->execute();
        std::cout << "Inserted user: " << username << std::endl;
        delete pstmt;
        return true;
    } catch (sql::SQLException& e) {
        if (e.getErrorCode() == 1062) { // MySQL error code for duplicate entry
            std::cerr << "Error inserting user: Username " << username << " already exists." << std::endl;
        }else
            std::cerr << "Error inserting user: " << e.what() << std::endl;
        return false;
    }
}

string MySQL::queryUserPassword(const std::string& username) {
    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("SELECT password FROM user_table WHERE username = '" + username + "'");
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next()) {
            std::string password = res->getString("password");
            delete res;
            delete pstmt;
            return password;
        } else {
            delete res;
            delete pstmt;
            return "NULL";
        }
    } catch (sql::SQLException& e) {
        std::cerr << "Error querying user password: " << e.what() << std::endl;
        return "NULL";
    }
}