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

AuthStatus UserManager::registerUser(const std::string& name, const std::string& username,
    const std::string& password, const std::string& phoneNumber,
    const std::string& email) {
    if (isUsernameTaken(username)) {
        return AuthStatus::UsernameTaken;
    }
    if (password.length() < 8) {
        return AuthStatus::PasswordTooShort;
    }
    if (!isValidPhoneNumber(phoneNumber)) {
        return AuthStatus::InvalidPhone;
    }
    if (!isValidEmail(email)) {
        return AuthStatus::InvalidEmail;
    }

    std::string securedPassword = hashPassword(password);
    User newUser(name, username, securedPassword, phoneNumber, email);
    users.push_back(newUser);

    return AuthStatus::Success;
}

AuthStatus UserManager::loginUser(const std::string& username, const std::string& password) const {
    std::string inputHash = hashPassword(password);
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            if (user.getPasswordHash() == inputHash) {
                return AuthStatus::Success;
            }
            else {
                return AuthStatus::IncorrectPassword;
            }
        }
    }
    return AuthStatus::UsernameNotFound;
}

AuthStatus UserManager::resetPasswordWithPhone(const std::string& username, const std::string& phone, const std::string& newPassword) {
    if (newPassword.length() < 8) {
        return AuthStatus::PasswordTooShort;
    }
    for (auto& user : users) {
        if (user.getUsername() == username) {
            if (user.getPhoneNumber() == phone) {
                user.setPasswordHash(hashPassword(newPassword));
                return AuthStatus::Success;
            }
            else {
                return AuthStatus::PhoneMismatch;
            }
        }
    }
    return AuthStatus::UsernameNotFound;
}

AuthStatus UserManager::updateUserProfile(const std::string& currentUsername, const std::string& newName,
    const std::string& newUsername, const std::string& newPassword,
    const std::string& newPhone, const std::string& newEmail) {
    if (currentUsername != newUsername && isUsernameTaken(newUsername)) {
        return AuthStatus::UsernameTaken;
    }
    if (newPassword.length() < 8) {
        return AuthStatus::PasswordTooShort;
    }
    if (!isValidPhoneNumber(newPhone)) {
        return AuthStatus::InvalidPhone;
    }
    if (!isValidEmail(newEmail)) {
        return AuthStatus::InvalidEmail;
    }

    for (auto& user : users) {
        if (user.getUsername() == currentUsername) {
            user.setName(newName);
            user.setUsername(newUsername);
            user.setPasswordHash(hashPassword(newPassword));
            user.setPhoneNumber(newPhone);
            user.setEmail(newEmail);
            return AuthStatus::Success;
        }
    }
    return AuthStatus::UsernameNotFound;
}

size_t UserManager::getUserCount() const {
    return users.size();
}