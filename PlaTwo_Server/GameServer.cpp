#include "GameServer.h"
#include <iostream>
#include <sstream>
#include <ctime>

using namespace std;

GameServer::GameServer(int portNum) : port(portNum), serverSocket(INVALID_SOCKET), isRunning(false) {}

GameServer::~GameServer() {
    stop();
}

vector<string> GameServer::splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool GameServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed.\n";
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed.\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    isRunning = true;
    cout << "Game Server is running on port " << port << "...\n";

    userManager.loadFromFile("users_data.txt");

    while (isRunning) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            clientThreads.emplace_back(&GameServer::handleClient, this, clientSocket);
        }
    }

    return true;
}

void GameServer::handleClient(SOCKET clientSocket) {
    char buffer[2048];
    string clientBuffer = "";

    while (isRunning) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            handleClientDisconnect(clientSocket);
            break;
        }

        buffer[bytesReceived] = '\0';
        clientBuffer += buffer;

        size_t pos;
        while ((pos = clientBuffer.find('\n')) != string::npos) {
            string requestStr = clientBuffer.substr(0, pos);
            clientBuffer.erase(0, pos + 1);

            if (!requestStr.empty() && requestStr.back() == '\r') {
                requestStr.pop_back();
            }

            if (requestStr.empty()) continue;

            NetworkPacket packet = NetworkPacket::deserialize(requestStr);
            string responseStr = processPacket(clientSocket, packet);

            if (!responseStr.empty()) {
                if (responseStr.back() != '\n') responseStr += "\n";
                send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()), 0);
            }
        }
    }
    closesocket(clientSocket);
}

string GameServer::processPacket(SOCKET clientSocket, const NetworkPacket& packet) {
    NetworkPacket response(PacketType::ERROR_MSG, "Server", "");
    bool sendResponse = true;

    switch (packet.getType()) {
    case PacketType::CONNECT_REQ:
        handleAuthAndConnect(clientSocket, packet, response);
        break;

    case PacketType::CREATE_ROOM:
        handleCreateRoom(clientSocket, packet, response);
        break;

    case PacketType::JOIN_ROOM:
        handleJoinRoom(clientSocket, packet, response);
        break;

    case PacketType::PAUSE_SAVE_REQ:
        handlePauseAndSave(clientSocket, packet, response);
        break;

    case PacketType::RECONNECT_REQ:
        handleReconnect(clientSocket, packet, response);
        break;

    case PacketType::MOVE_DOTS_BOXES:
    case PacketType::MOVE_NINE_MENS:
    case PacketType::MOVE_FANORONA:
    case PacketType::TURN_CHANGE:
    case PacketType::TIME_UP:
        forwardToOpponent(clientSocket, packet);
        sendResponse = false;
        break;

    case PacketType::GAME_OVER:
        handleGameOver(clientSocket, packet);
        forwardToOpponent(clientSocket, packet);
        sendResponse = false;
        break;

    default:
        response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Unsupported command");
        break;
    }

    if (sendResponse) {
        return response.serialize();
    }
    return "";
}

void GameServer::handleAuthAndConnect(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response) {
    lock_guard<mutex> lock(userMutex);

    vector<string> tokens = splitString(packet.getData(), '|');

    if (tokens.empty()) {
        response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Invalid format");
        return;
    }

    if (tokens[0] == "LOGIN" && tokens.size() >= 3) {
        userManager.loadFromFile("users_data.txt");
        AuthStatus status = userManager.loginUser(tokens[1], tokens[2]);
        if (status == AuthStatus::Success) {
            const User* loggedInUser = userManager.getUser(tokens[1]);
            string payload = "LOGIN_SUCCESS";

            if (loggedInUser) {
                payload += "|" + to_string(loggedInUser->getDotsAndBoxesScore()) +
                           "|" + to_string(loggedInUser->getNineMensMorrisScore()) +
                           "|" + to_string(loggedInUser->getFanoronaScore());

                const auto& history = loggedInUser->getGameHistory();
                payload += "|" + to_string(history.size());

                for (const auto& record : history) {
                    payload += "|" + to_string(static_cast<int>(record.gameName)) +
                               "|" + record.opponent +
                               "|" + record.date +
                               "|" + record.playerRole +
                               "|" + record.result +
                               "|" + to_string(record.score);
                }
            }
            response = NetworkPacket(PacketType::CONNECT_REQ, "Server", payload);
        }
        else {
            response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Login Failed");
        }
    }
    else if (tokens[0] == "REGISTER" && tokens.size() >= 6) {
        AuthStatus status = userManager.registerUser(tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
        if (status == AuthStatus::Success) {
            userManager.saveToFile("users_data.txt");
            response = NetworkPacket(PacketType::CONNECT_REQ, "Server", "REGISTER_SUCCESS");
        }
        else {
            response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Registration Failed");
        }
    }
    else if (tokens[0] == "FORGOT_PASS" && tokens.size() >= 4) {
        AuthStatus status = userManager.resetPasswordWithPhone(tokens[1], tokens[2], tokens[3]);
        if (status == AuthStatus::Success) {
            userManager.saveToFile("users_data.txt");
            response = NetworkPacket(PacketType::CONNECT_REQ, "Server", "RESET_SUCCESS");
        }
        else if (status == AuthStatus::PhoneMismatch) {
            response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Phone number mismatch");
        }
        else {
            response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Reset Password Failed");
        }
    }
    else {
        response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Unknown Auth Command");
    }
}

void GameServer::handleCreateRoom(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response) {
    lock_guard<mutex> lock(roomsMutex);
    vector<string> tokens = splitString(packet.getData(), '|');

    if (tokens.empty()) {
        response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Invalid room creation data");
        return;
    }

    string roomId = tokens[0];
    int boardSize = (tokens.size() >= 2) ? stoi(tokens[1]) : 6;
    int timeLimit = (tokens.size() >= 3) ? stoi(tokens[2]) : 0;

    GameRoom room;
    room.roomId = roomId;
    room.hostSocket = clientSocket;
    room.hostUsername = packet.getSender();
    room.boardSize = boardSize;
    room.timeLimitPerTurn = timeLimit;

    activeRooms[roomId] = room;
    response = NetworkPacket(PacketType::ROOM_JOINED, "Server", "Room created. Waiting for guest...");
}

void GameServer::handleJoinRoom(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response) {
    lock_guard<mutex> lock(roomsMutex);

    if (!activeRooms.empty()) {
        auto it = activeRooms.begin();
        GameRoom& room = it->second;

        if (!room.isGameStarted) {
            room.guestSocket = clientSocket;
            room.guestUsername = packet.getSender();
            room.isGameStarted = true;

            string configData = room.hostUsername + "|" + to_string(room.boardSize) + "|" + to_string(room.timeLimitPerTurn);
            response = NetworkPacket(PacketType::ROOM_JOINED, "Server", configData);

            NetworkPacket notifyHost(PacketType::GAME_START, "Server", room.guestUsername + "|" + to_string(room.boardSize) + "|" + to_string(room.timeLimitPerTurn));
            string msg = notifyHost.serialize();
            if (msg.back() != '\n') msg += "\n";
            send(room.hostSocket, msg.c_str(), static_cast<int>(msg.length()), 0);
            return;
        }
    }
    response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Room not found or game already started");
}

void GameServer::handlePauseAndSave(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response) {
    lock_guard<mutex> lock(roomsMutex);

    vector<string> tokens = splitString(packet.getData(), '|');
    if (tokens.size() >= 9) {
        SavedGame sg;
        sg.roomId = tokens[0];
        sg.gameType = static_cast<GameType>(stoi(tokens[1]));
        sg.hostUsername = tokens[2];
        sg.guestUsername = tokens[3];
        sg.hostColor = tokens[4];
        sg.guestColor = tokens[5];
        sg.currentTurnUsername = tokens[6];
        sg.remainingTime = stoi(tokens[7]);
        sg.gameStateData = tokens[8];

        lock_guard<mutex> uLock(userMutex);
        if (userManager.saveGameSession(sg)) {
            response = NetworkPacket(PacketType::PAUSE_SAVE_REQ, "Server", "SAVE_SUCCESS");
            forwardToOpponent(clientSocket, NetworkPacket(PacketType::PAUSE_SAVE_REQ, "Server", "GAME_PAUSED_BY_OPPONENT"));
            return;
        }
    }
    response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Save Failed");
}

void GameServer::handleReconnect(SOCKET clientSocket, const NetworkPacket& packet, NetworkPacket& response) {
    lock_guard<mutex> lock(roomsMutex);
    string roomId = packet.getData();

    SavedGame sg;
    lock_guard<mutex> uLock(userMutex);
    if (userManager.loadSavedGame(roomId, sg)) {
        string payload = sg.roomId + "|" + to_string(static_cast<int>(sg.gameType)) + "|"
            + sg.hostUsername + "|" + sg.guestUsername + "|"
            + sg.hostColor + "|" + sg.guestColor + "|"
            + sg.currentTurnUsername + "|" + to_string(sg.remainingTime) + "|"
            + sg.gameStateData;
        response = NetworkPacket(PacketType::RECONNECT_REQ, "Server", payload);
    }
    else {
        response = NetworkPacket(PacketType::ERROR_MSG, "Server", "Saved game not found");
    }
}

void GameServer::handleGameOver(SOCKET clientSocket, const NetworkPacket& packet) {
    lock_guard<mutex> lock(userMutex);

    vector<string> tokens = splitString(packet.getData(), '|');

    if (tokens.size() >= 7) {
        try {
            int gameTypeInt = stoi(tokens[0]);
            GameType gameType = static_cast<GameType>(gameTypeInt);

            string u1 = tokens[1];
            int s1 = stoi(tokens[2]);
            string r1 = tokens[3];

            string u2 = tokens[4];
            int s2 = stoi(tokens[5]);
            string r2 = tokens[6];

            time_t now = time(0);
            char dt[30];
            ctime_s(dt, sizeof(dt), &now);
            string dateStr(dt);
            if (!dateStr.empty() && dateStr.back() == '\n') dateStr.pop_back();

            userManager.loadFromFile("users_data.txt");

            User* user1 = const_cast<User*>(userManager.getUser(u1));
            if (user1) {
                user1->updateScore(gameType, s1);
                GameRecord rec1{ gameType, u2, dateStr, "Player", r1, s1 };
                user1->addGameRecord(rec1);
            }

            User* user2 = const_cast<User*>(userManager.getUser(u2));
            if (user2) {
                user2->updateScore(gameType, s2);
                GameRecord rec2{ gameType, u1, dateStr, "Player", r2, s2 };
                user2->addGameRecord(rec2);
            }

            userManager.saveToFile("users_data.txt");

        }
        catch (...) {
            cerr << "Error parsing GAME_OVER data format.\n";
        }
    }
}

void GameServer::forwardToOpponent(SOCKET clientSocket, const NetworkPacket& packet) {
    lock_guard<mutex> lock(roomsMutex);
    for (const auto& pair : activeRooms) {
        const GameRoom& room = pair.second;
        if (room.hostSocket == clientSocket || room.guestSocket == clientSocket) {
            SOCKET targetSocket = (clientSocket == room.hostSocket) ? room.guestSocket : room.hostSocket;
            if (targetSocket != INVALID_SOCKET) {
                string rawPacket = packet.serialize();
                if (rawPacket.back() != '\n') rawPacket += "\n";
                send(targetSocket, rawPacket.c_str(), static_cast<int>(rawPacket.length()), 0);
            }
            break;
        }
    }
}

void GameServer::handleClientDisconnect(SOCKET clientSocket) {
    lock_guard<mutex> lock(roomsMutex);

    auto it = activeRooms.begin();
    while (it != activeRooms.end()) {
        GameRoom& room = it->second;

        if (room.hostSocket == clientSocket || room.guestSocket == clientSocket) {
            if (!room.isGameStarted) {
                it = activeRooms.erase(it);
                cout << "A pending room was cleaned up due to host disconnection.\n";
                continue;
            }
            else {
                SOCKET opponentSocket = (room.hostSocket == clientSocket) ? room.guestSocket : room.hostSocket;

                if (room.hostSocket == clientSocket) room.hostSocket = INVALID_SOCKET;
                if (room.guestSocket == clientSocket) room.guestSocket = INVALID_SOCKET;

                if (opponentSocket != INVALID_SOCKET) {
                    NetworkPacket notify(PacketType::PAUSE_SAVE_REQ, "Server", "Opponent disconnected. Game paused.");
                    string msg = notify.serialize();
                    if (msg.back() != '\n') msg += "\n";
                    send(opponentSocket, msg.c_str(), static_cast<int>(msg.length()), 0);
                }
                ++it;
            }
        }
        else {
            ++it;
        }
    }
}

void GameServer::stop() {
    if (isRunning) {
        isRunning = false;
        closesocket(serverSocket);
        WSACleanup();
        for (auto& th : clientThreads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
}
