#pragma once
#pragma once
#include <string>
#include <vector>

using namespace std;

struct GameRecord {
    string gameName;      
    string opponent;      
    string date;          
    string playerRole;    
    string result;        
    int score;            
};

class User {
private:
    string name;          
    string username;      
    string passwordHash;  
    string phoneNumber;   
    string email;         

    int dotsAndBoxesScore;
    int nineMensMorrisScore;
    int fanoronaScore;

    vector<GameRecord> gameHistory;

public:
    User();
    User(string name, string username, string passwordHash, string phoneNumber, string email);

    string getName() const;
    string getUsername() const;
    string getPasswordHash() const;
    string getPhoneNumber() const;
    string getEmail() const;

    int getDotsAndBoxesScore() const;
    int getNineMensMorrisScore() const;
    int getFanoronaScore() const;
    const vector<GameRecord>& getGameHistory() const;

    void setName(const string& newName);
    void setUsername(const string& newUsername);
    void setPasswordHash(const string& newPasswordHash);
    void setPhoneNumber(const string& newPhoneNumber);
    void setEmail(const string& newEmail);

    void addGameRecord(const GameRecord& record);
    void updateScore(const string& gameName, int scoreChange);
};