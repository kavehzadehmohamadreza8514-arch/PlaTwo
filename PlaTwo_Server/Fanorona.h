#pragma once
#ifndef FANORONA_H
#define FANORONA_H

#include "BaseGame.h"
#include <vector>
#include <string>
#include <sstream>

class Fanorona : public BaseGame {
private:
    static const int ROWS = 5;
    static const int COLS = 9;
    static const int TOTAL_CELLS = 45;

    PlayerId board[TOTAL_CELLS];

    int pieceCount[3]; 

    bool waitingForChainCapture; 
    int chainPiecePos;           
    int lastDirRow, lastDirCol; 
    std::vector<int> visitedPositions; 

    static const int DIRECTIONS[8][2];

    void positionToRowCol(int pos, int& r, int& c) const;
    int rowColToPosition(int r, int c) const;

    bool isConnected(int from, int to) const;

    std::vector<int> getCaptureChain(int startPos, int dirRow, int dirCol, PlayerId opponent) const;

    bool hasCaptureFromPosition(int pos, PlayerId player, int forbiddenDirRow, int forbiddenDirCol,
        const std::vector<int>& visited) const;

    bool hasAnyCaptureAvailable(PlayerId player) const;

    bool hasAnyLegalMove(PlayerId player) const;

public:
    Fanorona(int timeLimitSeconds = 0);
    virtual ~Fanorona() = default;

    bool isValidMove(PlayerId player, const std::string& moveData) override;
    bool applyMove(PlayerId player, const std::string& moveData) override;
    bool checkGameOver() override;

    std::string getBoardStateString() const override;
    GameResult getResult() const override;

    std::string serializeState() const override;
    bool loadState(const std::string& stateData) override;
};

#endif