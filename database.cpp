#include "database.h"
#include "logger.h"
#include <thread>

std::mutex logMutex;
DatabaseManager::DatabaseManager() : henv(nullptr), hdbc(nullptr), hstmt(nullptr), ret(SQL_SUCCESS) {
    checkAndCreateDatabase();
}

DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::connectToDatabase() {
    hstmt = NULL;
    ret = SQL_SUCCESS;
    std::cout << "Connecting to the database..." << std::endl;
    std::unique_lock<std::mutex> lock(logMutex);
    logger.WriteLog("Connecting to the database...");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to allocate environment handle." << std::endl;
        logger.WriteLog("Failed to allocate environment handle.");
        return false;
    }

    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to set ODBC version." << std::endl;
        logger.WriteLog("Failed to set ODBC version.");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to allocate database connection handle." << std::endl;
        logger.WriteLog("Failed to allocate database connection handle.");
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::wstring connectionString = L"DSN=chatdb;UID=root;PWD=root";
    ret = SQLDriverConnect(hdbc, NULL, (SQLWCHAR*)connectionString.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);

        if (!checkAndCreateDatabase()) {
            std::cerr << "Failed to check and create database." << std::endl;
            logger.WriteLog("Failed to check and create database.");
            return false;
        }
    }

    std::cout << "Connected to the database." << std::endl;
    logger.WriteLog("Connected to the database.");

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DatabaseManager::disconnectFromDatabase() {
    if (hstmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
    }
    std::cout << "Disconnecting from the database..." << std::endl;
    logger.WriteLog("Disconnecting from the database...");

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (hstmt) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }

    if (hdbc) {
        SQLDisconnect(hdbc);
        std::cout << "Disconnected from the database." << std::endl;
        logger.WriteLog("Disconnected from the database.");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }

    if (henv) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}

bool DatabaseManager::createTables() {
    ret = SQL_SUCCESS;
    hstmt = NULL;

    if (connectToDatabase()) {
       
        std::string queryCreateUsers = "CREATE TABLE users ("
            "user_id INTEGER PRIMARY KEY AUTO_INCREMENT,"
            "first_name VARCHAR(50) NOT NULL,"
            "last_name VARCHAR(50) NOT NULL,"
            "email VARCHAR(100) UNIQUE NOT NULL"
            ");";
        std::unique_lock<std::mutex> lock(logMutex);
        logger.WriteLog("CREATE TABLE users.");
        std::string queryCreatePasswords = "CREATE TABLE passwords ("
            "user_id INTEGER PRIMARY KEY,"
            "password_hash VARCHAR(32) NOT NULL,"
            "FOREIGN KEY (user_id) REFERENCES users(user_id)"
            ");";
        logger.WriteLog("CREATE TABLE passwords.");
        std::string queryCreateMessages = "CREATE TABLE messages ("
            "message_id INTEGER PRIMARY KEY AUTO_INCREMENT,"
            "sender_id INTEGER NOT NULL,"
            "receiver_id INTEGER NOT NULL,"
            "message_text TEXT NOT NULL,"
            "send_date TIMESTAMP NOT NULL,"
            "delivery_status INTEGER NOT NULL DEFAULT 0,"
            "FOREIGN KEY (sender_id) REFERENCES users(user_id),"
            "FOREIGN KEY (receiver_id) REFERENCES users(user_id)"
            ");";
        logger.WriteLog("CREATE TABLE messages.");
        std::string queryCreateUserTrigger = "CREATE TRIGGER register_user_trigger\n"
            "AFTER INSERT ON users\n"
            "FOR EACH ROW\n"
            "BEGIN\n"
            "    INSERT INTO passwords (user_id, password_hash) VALUES (NEW.user_id, 'pass');\n"
            "END;";
        logger.WriteLog("CREATE TRIGGER register_user_trigger.");
        std::string queryCreateDeleteUserTrigger = "CREATE TRIGGER delete_user_trigger\n"
            "BEFORE DELETE ON users\n"
            "FOR EACH ROW\n"
            "BEGIN\n"
            "    DELETE FROM messages WHERE sender_id = OLD.user_id OR receiver_id = OLD.user_id;\n"
            "    DELETE FROM passwords WHERE user_id = OLD.user_id;\n"
            "END;";
        logger.WriteLog("CREATE TRIGGER delete_user_trigger.");
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryCreateUsers.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Failed to create 'users' table." << std::endl;
            logger.WriteLog("Failed to create 'users' table.");
            disconnectFromDatabase();
            return false;
        }
        std::cout << "Table 'users' created." << std::endl;
        logger.WriteLog("Table 'users' created.");

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryCreatePasswords.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Failed to create 'passwords' table." << std::endl;
            logger.WriteLog("Failed to create 'passwords' table.");
            disconnectFromDatabase();
            return false;
        }
        std::cout << "Table 'passwords' created." << std::endl;
        logger.WriteLog("Table 'passwords' created.");

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryCreateMessages.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Failed to create 'messages' table." << std::endl;
            logger.WriteLog("Failed to create 'messages' table.");
            disconnectFromDatabase();
            return false;
        }
        std::cout << "Table 'messages' created." << std::endl;
        logger.WriteLog("Table 'messages' created.");

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryCreateUserTrigger.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Failed to create 'register_user_trigger' trigger." << std::endl;
            logger.WriteLog("Failed to create 'register_user_trigger' trigger.");
            disconnectFromDatabase();
            return false;
        }
        std::cout << "Trigger 'register_user_trigger' created." << std::endl;
        logger.WriteLog("Trigger 'register_user_trigger' created.");

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryCreateDeleteUserTrigger.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::cerr << "Failed to create 'delete_user_trigger' trigger." << std::endl;
            logger.WriteLog("Failed to create 'delete_user_trigger' trigger.");
            disconnectFromDatabase();
            return false;
        }
        std::cout << "Trigger 'delete_user_trigger' created." << std::endl;
        logger.WriteLog("Trigger 'delete_user_trigger' created.");

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        disconnectFromDatabase();
        return true;
    }
    else {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }
    system("cls");

}

bool DatabaseManager::insertDataIntoTable() {
    if (connectToDatabase()) {
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

        std::string queryUsers = "INSERT INTO users(first_name, last_name, email) VALUES "
            "('User1', 'User1 Last', 'user1@example.com'),"
            "('User2', 'User2 Last', 'user2@example.com'),"
            "('User3', 'User3 Last', 'user3@example.com')";

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryUsers.c_str(), SQL_NTS);

        SQLLEN rowCount;
        SQLRowCount(hstmt, &rowCount);

        if (ret == SQL_SUCCESS && rowCount > 0) {
            std::cout << "Data inserted into 'users' table." << std::endl;
            logger.WriteLog("Data inserted into 'users' table.");
        }
        else {
            std::cerr << "Failed to insert data into 'users' table." << std::endl;
            logger.WriteLog("Failed to insert data into 'users' table.");
            disconnectFromDatabase();
            return false;
        }

        std::string queryMessages = "INSERT INTO messages(sender_id, receiver_id, message_text, send_date, delivery_status) VALUES "
            "(1, 2, 'Hello all', CURRENT_TIMESTAMP, 1),"
            "(2, 1, 'Hi', CURRENT_TIMESTAMP, 1),"
            "(3, 1, 'How are you?', CURRENT_TIMESTAMP, 0)";

        ret = SQLExecDirectA(hstmt, (SQLCHAR*)queryMessages.c_str(), SQL_NTS);

        SQLRowCount(hstmt, &rowCount);

        if (ret == SQL_SUCCESS && rowCount > 0) {
            std::cout << "Data inserted into 'messages' table." << std::endl;
            logger.WriteLog("Data inserted into 'messages' table.");
        }
        else {
            std::cerr << "Failed to insert data into 'messages' table." << std::endl;
            logger.WriteLog("Failed to insert data into 'messages' table.");
            disconnectFromDatabase();
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        disconnectFromDatabase();
        return true;
    }
    else {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }
}

bool DatabaseManager::checkAndCreateDatabase() {
    std::wcout << L"Initializing ODBC environment..." << std::endl;
    logger.WriteLog("Initializing ODBC environment...");
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

    std::wcout << L"Connecting to MySQL server..." << std::endl;
    logger.WriteLog("Connecting to MySQL server...");
    SQLWCHAR connStr[] = L"DRIVER={MySQL ODBC 8.0 ANSI Driver};"
        L"SERVER=localhost;"
        L"USER=root;"
        L"PASSWORD=root;"
        L"OPTION=3;";

    ret = SQLDriverConnectW(hdbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::wcerr << L"Failed to connect to the MySQL server." << std::endl;
        logger.WriteLog("Failed to connect to the MySQL server.");
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    std::wstring checkDbQuery = L"SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'chatdb'";
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    ret = SQLExecDirectW(hstmt, (SQLWCHAR*)checkDbQuery.c_str(), SQL_NTS);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::wcerr << L"Failed to execute the database existence check query." << std::endl;
        logger.WriteLog("Failed to execute the database existence check query.");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return false;
    }

    SQLLEN rowCount;
    SQLRowCount(hstmt, &rowCount);

    if (rowCount == 0) {

        std::wstring createDbQuery = L"CREATE DATABASE chatdb";
        std::wcout << L"Creating 'chatdb' database..." << std::endl;
        logger.WriteLog("Creating 'chatdb' database...");
        ret = SQLExecDirectW(hstmt, (SQLWCHAR*)createDbQuery.c_str(), SQL_NTS);

        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            std::wcerr << L"Failed to create 'chatdb' database." << std::endl;
            logger.WriteLog("Failed to create 'chatdb' database.");
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            SQLDisconnect(hdbc);
            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
            return false;
        }

        std::wcout << L"Database 'chatdb' created." << std::endl;
        logger.WriteLog("Database 'chatdb' created.");
        createTables();
        insertDataIntoTable();
    }
    else {
        std::wcout << L"Connection to database 'chatdb' established." << std::endl;
        logger.WriteLog("Connection to database 'chatdb' established.");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return true;
}