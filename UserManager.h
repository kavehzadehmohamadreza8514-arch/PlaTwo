#pragma once
#include <vector>
#include <string>
#include "User.h"

enum class AuthStatus {
    Success,
    UsernameTaken,
    InvalidEmail,
    InvalidPhone,
    PasswordTooShort,
    UsernameNotFound,
    IncorrectPassword,
    PhoneMismatch
};

class UserManager {
private:
    std::vector<User> users;

    bool isValidEmail(const std::string& email) const;
    bool isValidPhoneNumber(const std::string& phone) const;
    bool isUsernameTaken(const std::string& username) const;
    std::string hashPassword(const std::string& password) const;

public:
    UserManager() = default;

    AuthStatus registerUser(const std::string& name, const std::string& username,
        const std::string& password, const std::string& phoneNumber,
        const std::string& email);

    AuthStatus loginUser(const std::string& username, const std::string& password) const;

    AuthStatus resetPasswordWithPhone(const std::string& username, const std::string& phone, const std::string& newPassword);

    AuthStatus updateUserProfile(const std::string& currentUsername, const std::string& newName,
        const std::string& newUsername, const std::string& newPassword,
        const std::string& newPhone, const std::string& newEmail);

    size_t getUserCount() const;
};