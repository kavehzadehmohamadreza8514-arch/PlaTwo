#include "User.h"

using namespace std;

User::User() {
    name = "";
    username = "";
    passwordHash = "";
    phoneNumber = "";
    email = "";
    dotsAndBoxesScore = 0;
    nineMensMorrisScore = 0;
    fanoronaScore = 0;
}

User::User(string name, string username, string passwordHash, string phoneNumber, string email) {
    this->name = name;
    this->username = username;
    this->passwordHash = passwordHash;
    this->phoneNumber = phoneNumber;
    this->email = email;
    this->dotsAndBoxesScore = 0;
    this->nineMensMorrisScore = 0;
    this->fanoronaScore = 0;
}

string User::getName() const {
    return name;
}

string User::getUsername() const {
    return username;
}

string User::getPasswordHash() const {
    return passwordHash;
}

string User::getPhoneNumber() const {
    return phoneNumber;
}

string User::getEmail() const {
    return email;
}

int User::getDotsAndBoxesScore() const {
    return dotsAndBoxesScore;
}

int User::getNineMensMorrisScore() const {
    return nineMensMorrisScore;
}

int User::getFanoronaScore() const {
    return fanoronaScore;
}

const vector<GameRecord>& User::getGameHistory() const {
    return gameHistory;
}

void User::setName(const string& newName) {
    name = newName;
}

void User::setUsername(const string& newUsername) {
    username = newUsername;
}

void User::setPasswordHash(const string& newPasswordHash) {
    passwordHash = newPasswordHash;
}

void User::setPhoneNumber(const string& newPhoneNumber) {
    phoneNumber = newPhoneNumber;
}

void User::setEmail(const string& newEmail) {
    email = newEmail;
}

void User::addGameRecord(const GameRecord& record) {
    gameHistory.push_back(record);
}

void User::updateScore(GameType gameName, int scoreChange) {
    if (gameName == GameType::DotsAndBoxes) {
        dotsAndBoxesScore += scoreChange;
    }
    else if (gameName == GameType::NineMensMorris) {
        nineMensMorrisScore += scoreChange;
    }
    else if (gameName == GameType::Fanorona) {
        fanoronaScore += scoreChange;
    }
}
