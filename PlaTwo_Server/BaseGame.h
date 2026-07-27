#pragma once
#ifndef BASEGAME_H
#define BASEGAME_H

#include <string>

enum class GameType {
    DotsAndBoxes,
    NineMensMorris,
    Fanorona
};

enum class PlayerId {
    NONE = 0,
    PLAYER_1 = 1,
    PLAYER_2 = 2
};

struct GameResult {
    bool finished = false;
    bool isDraw = false;
    PlayerId winner = PlayerId::NONE;
    int p1Score = 0;
    int p2Score = 0;
    std::string reason;
};

class BaseGame {
protected:
    GameType gameType;
    PlayerId currentTurn;
    PlayerId winnerPlayer;
    bool isGameFinished;

    int timeLimitPerTurn;
    int p1RemainingTime;
    int p2RemainingTime;

public:
    BaseGame(GameType type, int timeLimitSeconds = 0)
        : gameType(type),
        currentTurn(PlayerId::PLAYER_1),
        winnerPlayer(PlayerId::NONE),
        isGameFinished(false),
        timeLimitPerTurn(timeLimitSeconds),
        p1RemainingTime(timeLimitSeconds),
        p2RemainingTime(timeLimitSeconds) {
    }

    virtual ~BaseGame() = default;
    virtual bool isValidMove(PlayerId player, const std::string& moveData) = 0;

    virtual bool applyMove(PlayerId player, const std::string& moveData) = 0;

    virtual bool checkGameOver() = 0;

    virtual std::string getBoardStateString() const = 0;

    virtual GameResult getResult() const = 0;

    virtual std::string serializeState() const = 0;
    virtual bool loadState(const std::string& stateData) = 0;

    GameType getGameType() const { return gameType; }
    PlayerId getCurrentTurn() const { return currentTurn; }

    void switchTurn() {
        currentTurn = (currentTurn == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
        p1RemainingTime = timeLimitPerTurn;
        p2RemainingTime = timeLimitPerTurn;
    }

    bool isFinished() const { return isGameFinished; }
    PlayerId getWinner() const { return winnerPlayer; }
    int getTimeLimit() const { return timeLimitPerTurn; }

    bool handleTimeout(PlayerId timedOutPlayer) {
        if (isGameFinished) return false;

        isGameFinished = true;
        winnerPlayer = (timedOutPlayer == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
        return true;
    }
};

#endif
