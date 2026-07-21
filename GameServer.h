#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <string>
#include "UserManager.h"
#include "NetworkPacket.h"

#pragma comment(lib, "Ws2_32.lib")

struct GameRoom {
    std::string roomId;
    SOCKET hostSocket = INVALID_SOCKET;
    SOCKET guestSocket = INVALID_SOCKET;
    std::string hostUsername;
    std::string guestUsername;
    bool isGameStarted = false;
};

class GameServer {
private:
    SOCKET serverSocket;
    int port;
    std::atomic<bool> isRunning;

    UserManager userManager;
    std::mutex userMutex;

    std::vector<std::thread> clientThreads;
    std::map<std::string, GameRoom> activeRooms;
    std::mutex roomsMutex;

    void handleClient(SOCKET clientSocket);
    std::string processPacket(SOCKET clientSocket, const NetworkPacket& packet);

    void handleAuthAndConnect(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response);
    void handleCreateRoom(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response);
    void handleJoinRoom(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response);

    void handleGameOver(SOCKET clientSocket, const NetworkPacket& packet);

    void handleClientDisconnect(SOCKET clientSocket);

    void forwardToOpponent(SOCKET clientSocket, const NetworkPacket& packet);

    std::vector<std::string> splitString(const std::string& str, char delimiter);

public:
    GameServer(int portNum = 8080);
    ~GameServer();

    bool start();
    void stop();
};