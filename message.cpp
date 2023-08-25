#include "message.h"
#include "database.h"
#include "logger.h"

extern SQLRETURN ret;
extern SQLHANDLE henv;
extern SQLHANDLE hdbc;
extern SQLHANDLE hstmt;

MessageManager::MessageManager() {
    DatabaseManager dbManager;
    if (dbManager.connectToDatabase()) {
        hdbc = dbManager.getHDBC();
        ret = SQL_SUCCESS;
        hstmt = NULL;
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    }
}

MessageManager::~MessageManager() {
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

bool MessageManager::sendMessage(const std::string& senderFirstName, const std::string& receiverFirstName, const std::string& messageText) {
    DatabaseManager dbManager;
    if (dbManager.connectToDatabase()) {

        std::string queryGetSenderID = "SELECT user_id FROM users WHERE first_name = ?";
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        ret = SQLPrepareA(hstmt, (SQLCHAR*)queryGetSenderID.c_str(), SQL_NTS);
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)senderFirstName.c_str(), 0, NULL);
        ret = SQLExecute(hstmt);

        SQLLEN senderID = -1; 
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            SQLGetData(hstmt, 1, SQL_C_LONG, &senderID, sizeof(senderID), NULL);
        }
        else {
            std::cerr << "Failed to retrieve sender ID." << std::endl;
            logger.WriteLog("Failed to retrieve sender ID.");
            dbManager.disconnectFromDatabase();
            return false;
        }

        std::string queryGetReceiverID = "SELECT user_id FROM users WHERE first_name = ?";

        ret = SQLPrepareA(hstmt, (SQLCHAR*)queryGetReceiverID.c_str(), SQL_NTS);
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)receiverFirstName.c_str(), 0, NULL);
        ret = SQLExecute(hstmt);

        SQLLEN receiverID = -1;
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            SQLGetData(hstmt, 1, SQL_C_LONG, &receiverID, sizeof(receiverID), NULL);
        }
        else {
            std::cerr << "Failed to retrieve receiver ID." << std::endl;
            logger.WriteLog("Failed to retrieve receiver ID.");
            dbManager.disconnectFromDatabase();
            return false;
        }

        std::string queryInsertMessage = "INSERT INTO messages(sender_id, receiver_id, message_text, send_date) VALUES (?, ?, ?, CURRENT_TIMESTAMP)";

        ret = SQLPrepareA(hstmt, (SQLCHAR*)queryInsertMessage.c_str(), SQL_NTS);
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &senderID, 0, NULL);
        ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &receiverID, 0, NULL);
        ret = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 1000, 0, (SQLCHAR*)messageText.c_str(), 0, NULL);
        ret = SQLExecute(hstmt);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            std::cout << "Message sent." << std::endl;
            logger.WriteLog("Message sent.");
        }
        else {
            std::cerr << "Failed to send message." << std::endl;
            logger.WriteLog("Failed to send message.");
            dbManager.disconnectFromDatabase();
            return false;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        dbManager.disconnectFromDatabase();
        return true;
    }
    else {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
        return false;
    }
}
