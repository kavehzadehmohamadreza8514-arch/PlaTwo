#pragma once
#include <vector>
#include <string>
#include "User.h"

class UserManager {
private:
    std::vector<User> users;

    bool isValidEmail(const std::string& email) const;
    bool isValidPhoneNumber(const std::string& phone) const;
    bool isUsernameTaken(const std::string& username) const;
    std::string hashPassword(const std::string& password) const;

public:
    UserManager() = default;

    bool registerUser(const std::string& name, const std::string& username,
        const std::string& password, const std::string& phoneNumber,
        const std::string& email);

    bool loginUser(const std::string& username, const std::string& password) const;

    bool resetPasswordWithPhone(const std::string& username, const std::string& phone, const std::string& newPassword);

    bool updateUserProfile(const std::string& currentUsername, const std::string& newName,
        const std::string& newUsername, const std::string& newPassword,
        const std::string& newPhone, const std::string& newEmail);

    size_t getUserCount() const;
};