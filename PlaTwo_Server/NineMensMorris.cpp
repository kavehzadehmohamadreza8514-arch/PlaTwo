#include "NineMensMorris.h"
#include <iostream>

using namespace std;

const int NineMensMorris::ADJACENCY[24][4] = {
    {1, 7, -1, -1},   
    {0, 2, 9, -1},    
    {1, 3, -1, -1},   
    {2, 4, 11, -1},  
    {3, 5, -1, -1},   
    {4, 6, 13, -1},   
    {5, 7, -1, -1},  
    {0, 6, 15, -1},   
    {9, 15, -1, -1},  
    {1, 8, 10, 17},   
    {9, 11, -1, -1},  
    {3, 10, 12, 19}, 
    {11, 13, -1, -1}, 
    {5, 12, 14, 21},  
    {13, 15, -1, -1}, 
    {7, 8, 14, 23},   
    {17, 23, -1, -1}, 
    {9, 16, 18, -1}, 
    {17, 19, -1, -1}, 
    {11, 18, 20, -1}, 
    {19, 21, -1, -1}, 
    {13, 20, 22, -1}, 
    {21, 23, -1, -1}, 
    {15, 16, 22, -1}  
};

const int NineMensMorris::MILLS[16][3] = {
    {0,1,2}, {2,3,4}, {4,5,6}, {6,7,0},       
    {8,9,10}, {10,11,12}, {12,13,14}, {14,15,8}, 
    {16,17,18}, {18,19,20}, {20,21,22}, {22,23,16}, 
    {1,9,17}, {3,11,19}, {5,13,21}, {7,15,23} 
};

NineMensMorris::NineMensMorris(int timeLimitSeconds)
    : BaseGame(GameType::NineMensMorris, timeLimitSeconds), waitingForCapture(false) {

    for (int i = 0; i < 24; ++i) {
        board[i] = PlayerId::NONE;
    }

    unplacedPieces[1] = 9; unplacedPieces[2] = 9;
    activePieces[1] = 0; activePieces[2] = 0;
}

bool NineMensMorris::isAdjacent(int from, int to) const {
    for (int i = 0; i < 4; ++i) {
        if (ADJACENCY[from][i] == to) return true;
    }
    return false;
}

bool NineMensMorris::isMill(int pos, PlayerId player) const {
    for (int i = 0; i < 16; ++i) {
        if (MILLS[i][0] == pos || MILLS[i][1] == pos || MILLS[i][2] == pos) {
            if (board[MILLS[i][0]] == player &&
                board[MILLS[i][1]] == player &&
                board[MILLS[i][2]] == player) {
                return true;
            }
        }
    }
    return false;
}

bool NineMensMorris::isAllInMill(PlayerId player) const {
    for (int i = 0; i < 24; ++i) {
        if (board[i] == player && !isMill(i, player)) {
            return false; 
        }
    }
    return true;
}

bool NineMensMorris::hasLegalMoves(PlayerId player) const {
    int pIdx = static_cast<int>(player);
    if (unplacedPieces[pIdx] > 0) return true; 
    if (activePieces[pIdx] <= 3) return true;  

    for (int i = 0; i < 24; ++i) {
        if (board[i] == player) {
            for (int j = 0; j < 4; ++j) {
                int adj = ADJACENCY[i][j];
                if (adj != -1 && board[adj] == PlayerId::NONE) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool NineMensMorris::isValidMove(PlayerId player, const string& moveData) {
    if (isGameFinished || currentTurn != player) return false;

    int pIdx = static_cast<int>(player);
    PlayerId opp = (player == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;

    stringstream ss(moveData);
    string typeStr;
    if (!getline(ss, typeStr, ',')) return false;
    char type = typeStr[0];

    if (type == 'C') {
        if (!waitingForCapture) return false;

        string posStr;
        if (!getline(ss, posStr, ',')) return false;
        int pos = stoi(posStr);

        if (pos < 0 || pos > 23 || board[pos] != opp) return false;

        if (isMill(pos, opp) && !isAllInMill(opp)) return false;

        return true;
    }
    else if (type == 'P') {
        if (waitingForCapture || unplacedPieces[pIdx] == 0) return false;

        string posStr;
        if (!getline(ss, posStr, ',')) return false;
        int pos = stoi(posStr);

        if (pos < 0 || pos > 23 || board[pos] != PlayerId::NONE) return false;
        return true;
    }
    else if (type == 'M') {
        if (waitingForCapture || unplacedPieces[pIdx] > 0) return false;

        string fromStr, toStr;
        if (!getline(ss, fromStr, ',') || !getline(ss, toStr, ',')) return false;
        int from = stoi(fromStr), to = stoi(toStr);

        if (from < 0 || from > 23 || to < 0 || to > 23) return false;
        if (board[from] != player || board[to] != PlayerId::NONE) return false;

        if (activePieces[pIdx] > 3 && !isAdjacent(from, to)) return false;

        return true;
    }

    return false;
}

bool NineMensMorris::applyMove(PlayerId player, const string& moveData) {
    if (!isValidMove(player, moveData)) return false;

    int pIdx = static_cast<int>(player);
    PlayerId opp = (player == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
    int oppIdx = static_cast<int>(opp);
    bool millFormed = false;

    stringstream ss(moveData);
    string typeStr;
    getline(ss, typeStr, ',');
    char type = typeStr[0];

    if (type == 'C') {
        string posStr;
        getline(ss, posStr, ',');
        int pos = stoi(posStr);

        board[pos] = PlayerId::NONE;
        activePieces[oppIdx]--;
        waitingForCapture = false;

    }
    else if (type == 'P') {
        string posStr;
        getline(ss, posStr, ',');
        int pos = stoi(posStr);

        board[pos] = player;
        unplacedPieces[pIdx]--;
        activePieces[pIdx]++;
        millFormed = isMill(pos, player);

    }
    else if (type == 'M') {
        string fromStr, toStr;
        getline(ss, fromStr, ',');
        getline(ss, toStr, ',');
        int from = stoi(fromStr), to = stoi(toStr);

        board[from] = PlayerId::NONE;
        board[to] = player;
        millFormed = isMill(to, player);
    }

    if (millFormed) {
        waitingForCapture = true;
        return true;
    }

    checkGameOver();
    if (!isGameFinished) {
        switchTurn();
    }
    return true;
}

bool NineMensMorris::checkGameOver() {
    if (isGameFinished) return true;

    bool p1Loses = (unplacedPieces[1] == 0) && (activePieces[1] < 3 || !hasLegalMoves(PlayerId::PLAYER_1));
    bool p2Loses = (unplacedPieces[2] == 0) && (activePieces[2] < 3 || !hasLegalMoves(PlayerId::PLAYER_2));

    if (p1Loses) {
        isGameFinished = true;
        winnerPlayer = PlayerId::PLAYER_2;
        return true;
    }
    if (p2Loses) {
        isGameFinished = true;
        winnerPlayer = PlayerId::PLAYER_1;
        return true;
    }

    return false;
}

GameResult NineMensMorris::getResult() const {
    GameResult result;
    result.finished = isGameFinished;
    result.winner = winnerPlayer;
    result.isDraw = (isGameFinished && winnerPlayer == PlayerId::NONE);
    result.p1Score = activePieces[1]; 
    result.p2Score = activePieces[2];
    result.reason = isGameFinished ? "Blocked or less than 3 pieces" : "In progress";
    return result;
}

string NineMensMorris::getBoardStateString() const {
    stringstream ss;
    for (int i = 0; i < 24; ++i) {
        ss << static_cast<int>(board[i]);
    }
    return ss.str();
}

string NineMensMorris::serializeState() const {
    stringstream ss;
    ss << static_cast<int>(currentTurn) << ","
        << (waitingForCapture ? 1 : 0) << ","
        << unplacedPieces[1] << "," << unplacedPieces[2] << ","
        << activePieces[1] << "," << activePieces[2];

    for (int i = 0; i < 24; ++i) {
        ss << "," << static_cast<int>(board[i]);
    }
    return ss.str();
}

bool NineMensMorris::loadState(const string& stateData) {
    vector<string> tokens;
    stringstream ss(stateData);
    string token;

    while (getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 30) return false;

    try {
        currentTurn = static_cast<PlayerId>(stoi(tokens[0]));
        waitingForCapture = (stoi(tokens[1]) == 1);
        unplacedPieces[1] = stoi(tokens[2]);
        unplacedPieces[2] = stoi(tokens[3]);
        activePieces[1] = stoi(tokens[4]);
        activePieces[2] = stoi(tokens[5]);

        for (int i = 0; i < 24; ++i) {
            board[i] = static_cast<PlayerId>(stoi(tokens[6 + i]));
        }
        return true;
    }
    catch (...) {
        return false;
    }
}
