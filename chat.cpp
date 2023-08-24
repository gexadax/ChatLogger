#include "chat.h"
#include "database.h"
#include <iostream>
#include "users.h"
#include "message.h"

SQLRETURN ret;
SQLHANDLE henv;
SQLHANDLE hdbc;
SQLHANDLE hstmt;

MessageManager messageManager;
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
                dbManager.disconnectFromDatabase();
                return;
            }

            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            dbManager.disconnectFromDatabase();
        }
        else {
            std::cerr << "Failed to connect to the database." << std::endl;
        }

    }

void userMenu(const std::string& username) {
    MessageManager messageManager;
    ChatManager chatManager;
    int userChoice;

    do {
        std::cout << "(User menu) 1. Send message 2. Read message 3. Delete user 4. Exit" << std::endl;
        std::cin >> userChoice;

        switch (userChoice) {
        case 1: {
            // ����� "Send message"
            std::string receiverFirstName, messageText;
            std::cout << "Enter the recipient's first name: ";
            std::cin >> receiverFirstName;
            std::cout << "Enter the message: ";
            std::cin.ignore(); // ����� ���������� ������ ����� ������ ����� ����������� �����
            std::getline(std::cin, messageText);

            if (messageManager.sendMessage(username, receiverFirstName, messageText)) {
                std::cout << "Message sent." << std::endl;
            }
            else {
                std::cout << "Failed to send message." << std::endl;
            }
            break;
        }
        case 2: {
            // ����� "Read message"
            chatManager.displayUserChat(username);
            break;
        }
        case 3: {
            // ����� "Delete user"
            std::string first_name;
            std::cout << "Enter the first name of the user to delete: ";
            std::cin >> first_name;
            UserManager userManager;
            if (userManager.deleteUserAndMessages(first_name)) {
                std::cout << "User and related messages deleted successfully." << std::endl;
            }
            else {
                std::cout << "Failed to delete user and related messages." << std::endl;
            }
            break;
        }

        case 4: {
            // ����� "Exit"
            std::cout << "Logging out." << std::endl;
            return; // ����� �� ����������������� ����
        }
        default: {
            std::cout << "Invalid choice. Please select a valid option." << std::endl;
            break;
        }
        }
    } while (userChoice != 4);
}

void chatRoom(const std::string& first_name) {
    int choice;

    do {
        std::cout << "Chat Room Options:" << std::endl;
        std::cout << "1. Send Message" << std::endl;
        std::cout << "2. Read Messages" << std::endl;
        std::cout << "3. Exit Chat Room" << std::endl;

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
            }
            else {
                std::cout << "Failed to send message." << std::endl;
            }
            break;
        }
        case 2: {
            ChatManager chatManager;
            chatManager.displayUserChat(first_name);
            break;
        }
        case 3: {
            std::cout << "Exiting Chat Room." << std::endl;
            return;
        }
        default: {
            std::cout << "Invalid choice. Please select a valid option." << std::endl;
            break;
        }
        }
    } while (choice != 3);
}

void chatMenu() {
    UserManager userManager;
    std::string first_name, password_hash;
    std::cout << "Enter your first name: "; std::cin >> first_name;
    std::cout << "Enter your password hash: "; std::cin >> password_hash;

    if (userManager.loginPass(first_name, password_hash)) {
        std::cout << "Login successful. Welcome, " << first_name << "!" << std::endl;
        chatRoom(first_name); // ����� ������� chatRoom ��� ��������������� ������������
    }
    else {
        std::cout << "Login failed. Please check your credentials." << std::endl;
    }
}



