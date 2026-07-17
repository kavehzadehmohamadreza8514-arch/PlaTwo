#include "UserManager.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <iomanip>

std::string UserManager::hashPassword(const std::string& password) const {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

bool UserManager::isUsernameTaken(const std::string& username) const {
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            return true;
        }
    }
    return false;
}

bool UserManager::isValidEmail(const std::string& email) const {
    const std::regex emailPattern(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, emailPattern);
}

bool UserManager::isValidPhoneNumber(const std::string& phone) const {
    const std::regex phonePattern(R"(09[0-9]{9})");
    return std::regex_match(phone, phonePattern);
}

bool UserManager::registerUser(const std::string& name, const std::string& username,
    const std::string& password, const std::string& phoneNumber,
    const std::string& email) {
    if (isUsernameTaken(username)) {
        std::cout << "Error: Username is already taken!" << std::endl;
        return false;
    }
    if (password.length() < 8) {
        std::cout << "Error: Password must be at least 8 characters long!" << std::endl;
        return false;
    }
    if (!isValidPhoneNumber(phoneNumber)) {
        std::cout << "Error: Invalid phone number format!" << std::endl;
        return false;
    }
    if (!isValidEmail(email)) {
        std::cout << "Error: Invalid email format!" << std::endl;
        return false;
    }

    std::string securedPassword = hashPassword(password);
    User newUser(name, username, securedPassword, phoneNumber, email);
    users.push_back(newUser);

    std::cout << "Success: User registered successfully!" << std::endl;
    return true;
}

bool UserManager::loginUser(const std::string& username, const std::string& password) const {
    std::string inputHash = hashPassword(password);
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            if (user.getPasswordHash() == inputHash) {
                std::cout << "Success: Welcome back!" << std::endl;
                return true;
            }
            else {
                std::cout << "Error: Incorrect password!" << std::endl;
                return false;
            }
        }
    }
    std::cout << "Error: Username not found!" << std::endl;
    return false;
}

bool UserManager::resetPasswordWithPhone(const std::string& username, const std::string& phone, const std::string& newPassword) {
    if (newPassword.length() < 8) {
        std::cout << "Error: New password must be at least 8 characters long!" << std::endl;
        return false;
    }
    for (auto& user : users) {
        if (user.getUsername() == username) {
            if (user.getPhoneNumber() == phone) {
                user.setPasswordHash(hashPassword(newPassword));
                std::cout << "Success: Password updated!" << std::endl;
                return true;
            }
            else {
                std::cout << "Error: Phone number does not match!" << std::endl;
                return false;
            }
        }
    }
    std::cout << "Error: Username not found!" << std::endl;
    return false;
}

bool UserManager::updateUserProfile(const std::string& currentUsername, const std::string& newName,
    const std::string& newUsername, const std::string& newPassword,
    const std::string& newPhone, const std::string& newEmail) {
    if (currentUsername != newUsername && isUsernameTaken(newUsername)) {
        std::cout << "Error: New username taken!" << std::endl;
        return false;
    }
    if (newPassword.length() < 8) {
        std::cout << "Error: New password too short!" << std::endl;
        return false;
    }
    if (!isValidPhoneNumber(newPhone)) {
        std::cout << "Error: Invalid phone!" << std::endl;
        return false;
    }
    if (!isValidEmail(newEmail)) {
        std::cout << "Error: Invalid email!" << std::endl;
        return false;
    }

    for (auto& user : users) {
        if (user.getUsername() == currentUsername) {
            user.setName(newName);
            user.setUsername(newUsername);
            user.setPasswordHash(hashPassword(newPassword));
            user.setPhoneNumber(newPhone);
            user.setEmail(newEmail);
            std::cout << "Success: Profile updated!" << std::endl;
            return true;
        }
    }
    return false;
}

size_t UserManager::getUserCount() const {
    return users.size();
}