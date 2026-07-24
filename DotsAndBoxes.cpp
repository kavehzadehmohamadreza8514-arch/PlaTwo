#include "DotsAndBoxes.h"
#include <iostream>
#include <vector>

using namespace std;

DotsAndBoxes::DotsAndBoxes(int size, int timeLimitSeconds)
    : BaseGame(GameType::DotsAndBoxes, timeLimitSeconds), boardSize(size), p1Score(0), p2Score(0) {

    if (boardSize < 6) boardSize = 6;
    if (boardSize > 8) boardSize = 8;

    horizontalLines.assign(boardSize, vector<bool>(boardSize - 1, false));
    verticalLines.assign(boardSize - 1, vector<bool>(boardSize, false));
    boxes.assign(boardSize - 1, vector<PlayerId>(boardSize - 1, PlayerId::NONE));
}

bool DotsAndBoxes::isValidMove(PlayerId player, const string& moveData) {
    if (isGameFinished || currentTurn != player) return false;

    if (moveData.length() < 5) return false;

    char type = moveData[0];
    if (type != 'H' && type != 'V') return false;

    stringstream ss(moveData.substr(2));
    int r, c;
    if (!(ss >> r >> c)) return false;

    if (type == 'H') {
        if (r < 0 || r >= boardSize || c < 0 || c >= boardSize - 1) return false;
        return !horizontalLines[r][c];
    }
    else {
        if (r < 0 || r >= boardSize - 1 || c < 0 || c >= boardSize) return false;
        return !verticalLines[r][c];
    }
}

bool DotsAndBoxes::applyMove(PlayerId player, const string& moveData) {
    if (!isValidMove(player, moveData)) return false;

    char type = moveData[0];
    stringstream ss(moveData.substr(2));
    int r, c;
    ss >> r >> c;

    if (type == 'H') {
        horizontalLines[r][c] = true;
    }
    else {
        verticalLines[r][c] = true;
    }

    int boxesClaimed = checkAndClaimBoxes(r, c, type, player);

    if (boxesClaimed > 0) {
        if (player == PlayerId::PLAYER_1) p1Score += boxesClaimed;
        else p2Score += boxesClaimed;

        checkGameOver();
        return false;
    }

    switchTurn();
    checkGameOver();
    return true;
}

int DotsAndBoxes::checkAndClaimBoxes(int r, int c, char lineType, PlayerId player) {
    int claimed = 0;

    if (lineType == 'H') {
        if (r > 0 && boxes[r - 1][c] == PlayerId::NONE) {
            if (horizontalLines[r - 1][c] && verticalLines[r - 1][c] && verticalLines[r - 1][c + 1]) {
                boxes[r - 1][c] = player;
                claimed++;
            }
        }
        if (r < boardSize - 1 && boxes[r][c] == PlayerId::NONE) {
            if (horizontalLines[r + 1][c] && verticalLines[r][c] && verticalLines[r][c + 1]) {
                boxes[r][c] = player;
                claimed++;
            }
        }
    }
    else if (lineType == 'V') {
        if (c > 0 && boxes[r][c - 1] == PlayerId::NONE) {
            if (verticalLines[r][c - 1] && horizontalLines[r][c - 1] && horizontalLines[r + 1][c - 1]) {
                boxes[r][c - 1] = player;
                claimed++;
            }
        }
        if (c < boardSize - 1 && boxes[r][c] == PlayerId::NONE) {
            if (verticalLines[r][c + 1] && horizontalLines[r][c] && horizontalLines[r + 1][c]) {
                boxes[r][c] = player;
                claimed++;
            }
        }
    }
    return claimed;
}

bool DotsAndBoxes::checkGameOver() {
    int totalBoxes = (boardSize - 1) * (boardSize - 1);
    if (p1Score + p2Score == totalBoxes) {
        isGameFinished = true;
        if (p1Score > p2Score) winnerPlayer = PlayerId::PLAYER_1;
        else if (p2Score > p1Score) winnerPlayer = PlayerId::PLAYER_2;
        else winnerPlayer = PlayerId::NONE;
        return true;
    }
    return false;
}

GameResult DotsAndBoxes::getResult() const {
    GameResult result;
    result.finished = isGameFinished;
    result.winner = winnerPlayer;
    result.p1Score = p1Score;
    result.p2Score = p2Score;
    result.isDraw = (isGameFinished && winnerPlayer == PlayerId::NONE);
    result.reason = isGameFinished ? "All boxes claimed" : "In progress";
    return result;
}

string DotsAndBoxes::getBoardStateJson() const {
    stringstream ss;
    ss << boardSize << ",";

    for (int r = 0; r < boardSize; ++r)
        for (int c = 0; c < boardSize - 1; ++c)
            ss << (horizontalLines[r][c] ? "1" : "0");
    ss << ",";

    for (int r = 0; r < boardSize - 1; ++r)
        for (int c = 0; c < boardSize; ++c)
            ss << (verticalLines[r][c] ? "1" : "0");
    ss << ",";

    for (int r = 0; r < boardSize - 1; ++r) {
        for (int c = 0; c < boardSize - 1; ++c) {
            if (boxes[r][c] == PlayerId::NONE) ss << "0";
            else if (boxes[r][c] == PlayerId::PLAYER_1) ss << "1";
            else ss << "2";
        }
    }
    ss << "," << p1Score << "," << p2Score;
    return ss.str();
}

string DotsAndBoxes::serializeState() const {
    return getBoardStateJson() + "," + to_string(static_cast<int>(currentTurn));
}

bool DotsAndBoxes::loadState(const string& stateData) {
    vector<string> tokens;
    stringstream ss(stateData);
    string token;

    while (getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 7) return false;

    try {
        boardSize = stoi(tokens[0]);

        horizontalLines.assign(boardSize, vector<bool>(boardSize - 1, false));
        verticalLines.assign(boardSize - 1, vector<bool>(boardSize, false));
        boxes.assign(boardSize - 1, vector<PlayerId>(boardSize - 1, PlayerId::NONE));

        string hLines = tokens[1];
        int idx = 0;
        for (int r = 0; r < boardSize; ++r) {
            for (int c = 0; c < boardSize - 1; ++c) {
                if (idx < hLines.length()) {
                    horizontalLines[r][c] = (hLines[idx] == '1');
                    idx++;
                }
            }
        }

        string vLines = tokens[2];
        idx = 0;
        for (int r = 0; r < boardSize - 1; ++r) {
            for (int c = 0; c < boardSize; ++c) {
                if (idx < vLines.length()) {
                    verticalLines[r][c] = (vLines[idx] == '1');
                    idx++;
                }
            }
        }

        string boxData = tokens[3];
        idx = 0;
        for (int r = 0; r < boardSize - 1; ++r) {
            for (int c = 0; c < boardSize - 1; ++c) {
                if (idx < boxData.length()) {
                    if (boxData[idx] == '1') boxes[r][c] = PlayerId::PLAYER_1;
                    else if (boxData[idx] == '2') boxes[r][c] = PlayerId::PLAYER_2;
                    else boxes[r][c] = PlayerId::NONE;
                    idx++;
                }
            }
        }

        p1Score = stoi(tokens[4]);
        p2Score = stoi(tokens[5]);
        currentTurn = static_cast<PlayerId>(stoi(tokens[6]));

        return true;
    }
    catch (...) {
        return false;
    }
}