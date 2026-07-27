#pragma once
#include <string>
#include <vector>

enum class GameType {
    DotsAndBoxes,
    NineMensMorris,
    Fanorona
};

struct GameRecord {
    GameType gameName;    
    std::string opponent;
    std::string date;
    std::string playerRole;
    std::string result;
    int score;
};

class User {
private:
    std::string name;
    std::string username;
    std::string passwordHash;
    std::string phoneNumber;
    std::string email;

    int dotsAndBoxesScore;
    int nineMensMorrisScore;
    int fanoronaScore;

    std::vector<GameRecord> gameHistory;

public:
    User();
    User(std::string name, std::string username, std::string passwordHash, std::string phoneNumber, std::string email);

    std::string getName() const;
    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getPhoneNumber() const;
    std::string getEmail() const;

    int getDotsAndBoxesScore() const;
    int getNineMensMorrisScore() const;
    int getFanoronaScore() const;
    const std::vector<GameRecord>& getGameHistory() const;

    void setName(const std::string& newName);
    void setUsername(const std::string& newUsername);
    void setPasswordHash(const std::string& newPasswordHash);
    void setPhoneNumber(const std::string& newPhoneNumber);
    void setEmail(const std::string& newEmail);

    void addGameRecord(const GameRecord& record);
    void updateScore(GameType gameName, int scoreChange); 
};
