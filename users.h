#pragma once
#include <windows.h>
#include <sql.h>
#include <iostream>
#include <vector>

class UserManager {
    
public:
    UserManager();
    ~UserManager();

    bool registerUser(const std::string& first_name, const std::string& last_name, const std::string& email);
    bool deleteUserAndMessages(const std::string& first_name);
    bool loginPass(const std::string& first_name, const std::string& password_hash);
};


