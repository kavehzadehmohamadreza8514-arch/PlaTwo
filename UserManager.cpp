#include "UserManager.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream> 

using namespace std; 

string UserManager::hashPassword(const string& password) const {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    stringstream ss;
    ss << hex << setw(16) << setfill('0') << hash;
    return ss.str();
}

bool UserManager::isUsernameTaken(const string& username) const {
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            return true;
        }
    }
    return false;
}

bool UserManager::isValidEmail(const string& email) const {
    const std::regex emailPattern(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)"); 
    return std::regex_match(email, emailPattern);
}

bool UserManager::isValidPhoneNumber(const string& phone) const {
    const std::regex phonePattern(R"(09[0-9]{9})");
    return std::regex_match(phone, phonePattern);
}

AuthStatus UserManager::registerUser(const string& name, const string& username,
    const string& password, const string& phoneNumber,
    const string& email) {
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

    string securedPassword = hashPassword(password);
    User newUser(name, username, securedPassword, phoneNumber, email);
    users.push_back(newUser);

    return AuthStatus::Success;
}

AuthStatus UserManager::loginUser(const string& username, const string& password) const {
    string inputHash = hashPassword(password);
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

AuthStatus UserManager::resetPasswordWithPhone(const string& username, const string& phone, const string& newPassword) {
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

AuthStatus UserManager::updateUserProfile(const string& currentUsername, const string& newName,
    const string& newUsername, const string& newPassword,
    const string& newPhone, const string& newEmail) {
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


bool UserManager::saveToFile(const string& filename) const {
    ofstream outFile(filename);

    if (!outFile.is_open()) {
        return false;
    }

    outFile << users.size() << "\n";

    for (const auto& user : users) {
        outFile << user.getName() << "\n"
            << user.getUsername() << "\n"
            << user.getPasswordHash() << "\n"
            << user.getPhoneNumber() << "\n"
            << user.getEmail() << "\n"
            << user.getDotsAndBoxesScore() << "\n"
            << user.getNineMensMorrisScore() << "\n"
            << user.getFanoronaScore() << "\n";

        const auto& history = user.getGameHistory();
        outFile << history.size() << "\n";

        for (const auto& record : history) {
            outFile << static_cast<int>(record.gameName) << "\n"
                << record.opponent << "\n"
                << record.date << "\n"
                << record.playerRole << "\n"
                << record.result << "\n"
                << record.score << "\n";
        }
    }

    outFile.close();
    return true;
}

bool UserManager::loadFromFile(const string& filename) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        return false;
    }

    users.clear(); 

    string line;
    if (!getline(inFile, line)) return false;
    stringstream ss(line);
    size_t userCount = 0;
    ss >> userCount;

    for (size_t i = 0; i < userCount; ++i) {
        string name, username, passwordHash, phoneNumber, email;
        string scoreStr;
        int dotsScore = 0, nineScore = 0, fanoronaScore = 0;

        if (!getline(inFile, name)) break;
        if (!getline(inFile, username)) break;
        if (!getline(inFile, passwordHash)) break;
        if (!getline(inFile, phoneNumber)) break;
        if (!getline(inFile, email)) break;

        if (getline(inFile, scoreStr)) { stringstream(scoreStr) >> dotsScore; }
        if (getline(inFile, scoreStr)) { stringstream(scoreStr) >> nineScore; }
        if (getline(inFile, scoreStr)) { stringstream(scoreStr) >> fanoronaScore; }

        User user(name, username, passwordHash, phoneNumber, email);

        if (dotsScore > 0) user.updateScore(GameType::DotsAndBoxes, dotsScore);
        if (nineScore > 0) user.updateScore(GameType::NineMensMorris, nineScore);
        if (fanoronaScore > 0) user.updateScore(GameType::Fanorona, fanoronaScore);

        string historyCountStr;
        size_t historyCount = 0;
        if (getline(inFile, historyCountStr)) {
            stringstream(historyCountStr) >> historyCount;
        }

        for (size_t j = 0; j < historyCount; ++j) {
            string gameNameStr, opponent, date, playerRole, result, recordScoreStr;
            GameRecord record;

            if (!getline(inFile, gameNameStr)) break;
            int gameTypeInt = 0;
            stringstream(gameNameStr) >> gameTypeInt;
            record.gameName = static_cast<GameType>(gameTypeInt);

            if (!getline(inFile, opponent)) break;
            record.opponent = opponent;

            if (!getline(inFile, date)) break;
            record.date = date;

            if (!getline(inFile, playerRole)) break;
            record.playerRole = playerRole;

            if (!getline(inFile, result)) break;
            record.result = result;

            if (!getline(inFile, recordScoreStr)) break;
            stringstream(recordScoreStr) >> record.score;

            user.addGameRecord(record);
        }

        users.push_back(user);
    }

    inFile.close();
    return true;
}