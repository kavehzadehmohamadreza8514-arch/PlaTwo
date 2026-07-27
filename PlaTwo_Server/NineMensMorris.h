#pragma once
#ifndef NINEMENSMORRIS_H
#define NINEMENSMORRIS_H

#include "BaseGame.h"
#include <vector>
#include <string>
#include <sstream>

class NineMensMorris : public BaseGame {
private:
    PlayerId board[24];

    int unplacedPieces[3]; 
    int activePieces[3];   

    bool waitingForCapture;

    static const int ADJACENCY[24][4];
    static const int MILLS[16][3];

    bool isMill(int pos, PlayerId player) const;
    bool isAllInMill(PlayerId player) const;
    bool hasLegalMoves(PlayerId player) const;
    bool isAdjacent(int from, int to) const;

public:
    NineMensMorris(int timeLimitSeconds = 0);
    virtual ~NineMensMorris() = default;

    bool isValidMove(PlayerId player, const std::string& moveData) override;
    bool applyMove(PlayerId player, const std::string& moveData) override;
    bool checkGameOver() override;
    std::string getBoardStateString() const override;
    GameResult getResult() const override;
    std::string serializeState() const override;
    bool loadState(const std::string& stateData) override;
};

#endif
