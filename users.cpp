#include "users.h"
#include "database.h" 
#include "logger.h"

extern SQLRETURN ret;
extern SQLHANDLE henv;
extern SQLHANDLE hdbc;
extern SQLHANDLE hstmt;

UserManager::UserManager() {

    ret = SQL_SUCCESS;
    hstmt = NULL;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
 }

UserManager::~UserManager() {
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

bool UserManager::registerUser(const std::string& first_name, const std::string& last_name, const std::string& email) {
    DatabaseManager dbManager;
    if (!dbManager.connectToDatabase()) {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }

    SQLRETURN ret;
    SQLHANDLE hstmt;
    SQLHANDLE hdbc = dbManager.getHDBC();
    std::string queryInsertUser = "INSERT INTO users (first_name, last_name, email) VALUES (?, ?, ?)";

    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    ret = SQLPrepareA(hstmt, (SQLCHAR*)queryInsertUser.c_str(), SQL_NTS);
    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)first_name.c_str(), 0, NULL);
    ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)last_name.c_str(), 0, NULL);
    ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)email.c_str(), 0, NULL);
    ret = SQLExecute(hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to register user." << std::endl;
        logger.WriteLog("Failed to register user.");
        return false;
    }

    std::cout << "User registered successfully." << std::endl;
    logger.WriteLog("User registered successfully.");
}

bool UserManager::deleteUserAndMessages(const std::string& first_name) {
    DatabaseManager dbManager;

    if (!dbManager.connectToDatabase()) {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }

    SQLRETURN ret;
    SQLHANDLE hstmt;
    SQLHANDLE hdbc = dbManager.getHDBC();

    std::string queryDeleteUserAndMessages = "DELETE FROM users WHERE first_name = ?";
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    ret = SQLPrepareA(hstmt, (SQLCHAR*)queryDeleteUserAndMessages.c_str(), SQL_NTS);
    ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)first_name.c_str(), 0, NULL);

    ret = SQLExecute(hstmt);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cerr << "Failed to delete user and messages." << std::endl;
        logger.WriteLog("Failed to delete user and messages.");
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    return true;
}


bool UserManager::loginPass(const std::string& first_name, const std::string& password_hash) {
    DatabaseManager dbManager;

    if (dbManager.connectToDatabase()) {
        SQLHANDLE hstmt;
        SQLHANDLE hdbc = dbManager.getHDBC();

        std::string queryLogin = "SELECT u.user_id FROM users u "
            "INNER JOIN passwords p ON u.user_id = p.user_id "
            "WHERE u.first_name = ? AND p.password_hash = ?";

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        ret = SQLPrepareA(hstmt, (SQLCHAR*)queryLogin.c_str(), SQL_NTS);
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)first_name.c_str(), 0, NULL);
        ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0, (SQLCHAR*)password_hash.c_str(), 0, NULL);

        ret = SQLExecute(hstmt);

        SQLINTEGER user_id = 0;
        ret = SQLBindCol(hstmt, 1, SQL_C_SLONG, &user_id, sizeof(user_id), NULL);

        ret = SQLFetch(hstmt);
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            std::cout << "Retrieved user_id: " << user_id << std::endl;
            logger.WriteLog("Retrieved user_id: ");
            if (user_id > 0) {
                std::cout << "Login successful. User ID: " << user_id << std::endl;
                logger.WriteLog("Login successful. User ID: ");
                SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                return true;
            }
        }
        else {
            std::cerr << "Login failed." << std::endl;
            logger.WriteLog("Login failed.");
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    }
    else {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }

    return false;
}



