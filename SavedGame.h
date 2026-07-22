#pragma once
#ifndef SAVEDGAME_H
#define SAVEDGAME_H

#include <string>
#include "BaseGame.h"

struct SavedGame {
    std::string roomId;
    GameType gameType;
    std::string hostUsername;
    std::string guestUsername;
    std::string currentTurnUsername;
    std::string gameStateData; 
    int remainingTime;
};

#endif 