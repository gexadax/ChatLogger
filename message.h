#pragma once
#include <string>

class MessageManager {

public:

    MessageManager(); 
    ~MessageManager();
    bool sendMessage(const std::string& senderFirstName, const std::string& receiverFirstName, const std::string& messageText);
};
