#pragma once
#ifndef GAMESESSION_H
#define GAMESESSION_H

#include <string>
#include <memory>
#include <chrono>
#include "BaseGame.h"

class GameSession {
private:
    std::string sessionId;

    std::string player1Username;
    std::string player2Username;

    std::string player1Color;
    std::string player2Color;

    std::unique_ptr<BaseGame> gameLogic;

    bool p1Connected;
    bool p2Connected;

    std::chrono::steady_clock::time_point lastTurnStartTime;

public:
    GameSession(const std::string& id,
        const std::string& p1User, const std::string& p2User,
        BaseGame* game,
        const std::string& p1Color = "", const std::string& p2Color = "")
        : sessionId(id),
        player1Username(p1User),
        player2Username(p2User),
        player1Color(p1Color),
        player2Color(p2Color),
        gameLogic(game),
        p1Connected(true),
        p2Connected(true)
    {
        resetTurnTimer();
    }

    ~GameSession() = default;


    std::string getSessionId() const { return sessionId; }
    std::string getPlayer1Username() const { return player1Username; }
    std::string getPlayer2Username() const { return player2Username; }
    std::string getPlayer1Color() const { return player1Color; }
    std::string getPlayer2Color() const { return player2Color; }

    BaseGame* getGame() { return gameLogic.get(); }
    const BaseGame* getGame() const { return gameLogic.get(); }

    PlayerId getPlayerIdByUsername(const std::string& username) const {
        if (username == player1Username) return PlayerId::PLAYER_1;
        if (username == player2Username) return PlayerId::PLAYER_2;
        return PlayerId::NONE;
    }

    std::string getOpponentUsername(const std::string& username) const {
        if (username == player1Username) return player2Username;
        if (username == player2Username) return player1Username;
        return "";
    }


    void setPlayerConnected(PlayerId player, bool connected) {
        if (player == PlayerId::PLAYER_1) p1Connected = connected;
        if (player == PlayerId::PLAYER_2) p2Connected = connected;
    }

    bool isPlayerConnected(PlayerId player) const {
        if (player == PlayerId::PLAYER_1) return p1Connected;
        if (player == PlayerId::PLAYER_2) return p2Connected;
        return false;
    }

    bool isBothConnected() const {
        return p1Connected && p2Connected;
    }


    void resetTurnTimer() {
        lastTurnStartTime = std::chrono::steady_clock::now();
    }

    bool checkTimeout() {
        if (!gameLogic || gameLogic->getTimeLimit() <= 0 || gameLogic->isFinished()) {
            return false;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - lastTurnStartTime).count();

        if (elapsedSeconds >= gameLogic->getTimeLimit()) {
            gameLogic->handleTimeout(gameLogic->getCurrentTurn());
            return true;
        }

        return false;
    }

    int getRemainingSecondsForCurrentTurn() const {
        if (!gameLogic || gameLogic->getTimeLimit() <= 0) return 0;

        auto now = std::chrono::steady_clock::now();
        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - lastTurnStartTime).count();
        int remaining = gameLogic->getTimeLimit() - static_cast<int>(elapsedSeconds);

        return (remaining > 0) ? remaining : 0;
    }


    bool processMove(PlayerId player, const std::string& moveData) {
        if (!gameLogic || gameLogic->isFinished()) return false;

        if (gameLogic->getCurrentTurn() != player) return false;

        if (gameLogic->isValidMove(player, moveData)) {
            bool turnSwitched = gameLogic->applyMove(player, moveData);

            if (turnSwitched) {
                resetTurnTimer();
            }
            return true;
        }

        return false;
    }
};

#endif 