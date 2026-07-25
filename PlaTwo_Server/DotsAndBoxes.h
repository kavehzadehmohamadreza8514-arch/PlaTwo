#pragma once
#ifndef DOTSANDBOXES_H
#define DOTSANDBOXES_H

#include "BaseGame.h"
#include <vector>
#include <string>
#include <sstream>

class DotsAndBoxes : public BaseGame {
private:
    int boardSize;

    std::vector<std::vector<bool>> horizontalLines;

    std::vector<std::vector<bool>> verticalLines;

    std::vector<std::vector<PlayerId>> boxes;

    int p1Score;
    int p2Score;

    int checkAndClaimBoxes(int r, int c, char lineType, PlayerId player);

public:
    DotsAndBoxes(int size = 6, int timeLimitSeconds = 0);
    virtual ~DotsAndBoxes() = default;

    bool isValidMove(PlayerId player, const std::string& moveData) override;
    bool applyMove(PlayerId player, const std::string& moveData) override;
    bool checkGameOver() override;

    std::string getBoardStateString() const override;
    GameResult getResult() const override;

    std::string serializeState() const override;
    bool loadState(const std::string& stateData) override;
};

#endif