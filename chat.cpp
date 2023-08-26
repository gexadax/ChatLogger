#include <algorithm>

#include "chat.h"
#include "database.h"
#include <iostream>
#include <sstream>
#include "users.h"
#include "message.h"
#include "logger.h"

SQLRETURN ret;
SQLHANDLE henv;
SQLHANDLE hdbc;
SQLHANDLE hstmt;
MessageManager messageManager;

Logger logger("log.txt");

void ChatManager::displayUserChat(const std::string& username) {
    DatabaseManager dbManager;
    if (dbManager.connectToDatabase()) {
        SQLRETURN ret;
        SQLHANDLE hstmt;
        SQLHANDLE hdbc = dbManager.getHDBC();
        std::string queryGetChat = "SELECT u.first_name, m.message_text, m.send_date "
            "FROM messages m "
            "INNER JOIN users u ON m.sender_id = u.user_id "
            "WHERE u.first_name = ? "
            "ORDER BY m.send_date";

        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        ret = SQLPrepareA(hstmt, (SQLCHAR*)queryGetChat.c_str(), SQL_NTS);
        ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)username.c_str(), 0, NULL);

        ret = SQLExecute(hstmt);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            std::cout << "Chat history for user '" << username << "':" << std::endl;
            logger.WriteLog("Chat history for user");

            SQLCHAR senderName[50], message[1000], timestamp[50];
            SQLLEN senderNameLen, messageLen, timestampLen;

            while (SQLFetch(hstmt) == SQL_SUCCESS) {
                SQLGetData(hstmt, 1, SQL_C_CHAR, senderName, sizeof(senderName), &senderNameLen);
                SQLGetData(hstmt, 2, SQL_C_CHAR, message, sizeof(message), &messageLen);
                SQLGetData(hstmt, 3, SQL_C_CHAR, timestamp, sizeof(timestamp), &timestampLen);

                std::cout << timestamp << " " << senderName << ": " << message << std::endl;
            }

        }
        else {
            std::cerr << "Failed to retrieve chat history." << std::endl;
            logger.WriteLog("Failed to retrieve chat history.");
            dbManager.disconnectFromDatabase();
            return;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        dbManager.disconnectFromDatabase();
    }
    else {
        std::cerr << "Failed to connect to the database." << std::endl;
        logger.WriteLog("Failed to connect to the database.");
    }
}

void chatRoom(const std::string& first_name) {
    std::system("cls");
    int choice;

    do {
        
        std::cout << "Chat Room Options:" << std::endl;
        std::cout << "1. Send Message" << std::endl;
        std::cout << "2. Read Messages" << std::endl;
        std::cout << "3. Read Log" << std::endl;
        std::cout << "4. Delete User" << std::endl;
        std::cout << "5. Exit Chat Room" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice) {
        case 1: {
            std::string receiverFirstName, messageText;
            std::cout << "Enter the recipient's first name: ";
            std::cin >> receiverFirstName;
            std::cout << "Enter the message: ";
            std::cin.ignore();
            std::getline(std::cin, messageText);


            if (messageManager.sendMessage(first_name, receiverFirstName, messageText)) {
                std::cout << "Message sent." << std::endl;
                logger.WriteLog("Message sent.");
            }
            else {
                std::cout << "Failed to send message." << std::endl;
                logger.WriteLog("Failed to send message.");
            }
            break;
        }
        case 2: {
            ChatManager chatManager;
            chatManager.displayUserChat(first_name);
            break;
        }
        case 3: {
            std::string lastLogLines = logger.ReadLastLines(10);
            std::cout << "Last 10 log lines:\n" << lastLogLines << std::endl;

            std::string logContent = logger.ReadLog();
            std::cout << "Log Content:" << std::endl << logContent << std::endl;
            break;
        }



        case 4: {
            std::string first_name_to_delete;
            std::cout << "Enter the first name of the user to delete: ";
            std::cin >> first_name_to_delete;
            UserManager userManager;
            if (userManager.deleteUserAndMessages(first_name_to_delete)) {
                std::cout << "User and related messages deleted successfully." << std::endl;
                logger.WriteLog("User and related messages deleted successfully.");
            }
            else {
                std::cout << "Failed to delete user and related messages." << std::endl;
                logger.WriteLog("Failed to delete user and related messages.");
            }
            break;
        }
        case 5: {
            std::cout << "Exiting Chat Room." << std::endl;
            logger.WriteLog("Exiting Chat Room.");
            return;
        }
        default: {
            std::cout << "Invalid choice. Please select a valid option." << std::endl;
            logger.WriteLog("Invalid choice. Please select a valid option.");
            break;
        }
        }
    } while (true);
}

void chatMenu() {

    UserManager userManager;
    DatabaseManager dbManager;
    std::string first_name, password_hash;
    int userChoice;

    do {
        std::cout << "(Chat Menu) 1. Register 2. Login 3. Exit" << std::endl;
        std::cin >> userChoice;

        switch (userChoice) {
        case 1: {
            std::system("cls");
            std::string last_name, email;
            std::cout << "Enter your first name: "; std::cin >> first_name;
            std::cout << "Enter your last name: "; std::cin >> last_name;
            std::cout << "Enter your email: "; std::cin >> email;

            if (userManager.registerUser(first_name, last_name, email)) {
                std::cout << "Registration successful. Welcome, " << first_name << "!" << std::endl;
                logger.WriteLog("Registration successful. Welcome");
                chatRoom(first_name);
            }
            else {
                std::cout << "Registration failed. Please try again." << std::endl;
                logger.WriteLog("Registration failed. Please try again.");
            }
            break;
        }
        case 2: {
            std::cout << "Enter your first name: "; std::cin >> first_name;
            std::cout << "Enter your password hash: "; std::cin >> password_hash;

            if (userManager.loginPass(first_name, password_hash)) {
                std::cout << "Login successful. Welcome, " << first_name << "!" << std::endl;
                logger.WriteLog("Login successful. Welcome");
                chatRoom(first_name);
            }
            else {
                std::cout << "Login failed. Please check your credentials." << std::endl;
                logger.WriteLog("Login failed. Please check your credentials.");
            }
            break;
        }
        case 3: {
            std::cout << "Exiting the chat." << std::endl;
            logger.WriteLog("Exiting the chat.");
            dbManager.disconnectFromDatabase();

            return;
        }
        default: {
            std::cout << "Invalid choice. Please select a valid option." << std::endl;
            logger.WriteLog("Invalid choice. Please select a valid option.");
            break;
        }
        }
    } while (userChoice != 3);
}
