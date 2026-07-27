#include "Fanorona.h"

using namespace std;

const int Fanorona::DIRECTIONS[8][2] = {
    {-1,-1}, {-1,0}, {-1,1},
    { 0,-1},         { 0,1},
    { 1,-1}, { 1,0}, { 1,1}
};

Fanorona::Fanorona(int timeLimitSeconds)
    : BaseGame(GameType::Fanorona, timeLimitSeconds),
    waitingForChainCapture(false), chainPiecePos(-1), lastDirRow(0), lastDirCol(0) {

    for (int c = 0; c < COLS; ++c) {
        board[rowColToPosition(0, c)] = PlayerId::PLAYER_1;
        board[rowColToPosition(1, c)] = PlayerId::PLAYER_1;
        board[rowColToPosition(3, c)] = PlayerId::PLAYER_2;
        board[rowColToPosition(4, c)] = PlayerId::PLAYER_2;
    }

    PlayerId middleRow[COLS] = {
        PlayerId::PLAYER_2, PlayerId::PLAYER_1, PlayerId::PLAYER_2, PlayerId::PLAYER_1,
        PlayerId::NONE,
        PlayerId::PLAYER_1, PlayerId::PLAYER_2, PlayerId::PLAYER_1, PlayerId::PLAYER_2
    };
    for (int c = 0; c < COLS; ++c) {
        board[rowColToPosition(2, c)] = middleRow[c];
    }

    pieceCount[1] = 22;
    pieceCount[2] = 22;
}

void Fanorona::positionToRowCol(int pos, int& r, int& c) const {
    r = pos / COLS;
    c = pos % COLS;
}

int Fanorona::rowColToPosition(int r, int c) const {
    return r * COLS + c;
}

bool Fanorona::isConnected(int from, int to) const {
    int r1, c1, r2, c2;
    positionToRowCol(from, r1, c1);
    positionToRowCol(to, r2, c2);

    int dr = r2 - r1;
    int dc = c2 - c1;

    if (dr < -1 || dr > 1 || dc < -1 || dc > 1) return false;
    if (dr == 0 && dc == 0) return false;

    if (dr != 0 && dc != 0) {
        if ((r1 + c1) % 2 != 0) return false;
    }
    return true;
}

vector<int> Fanorona::getCaptureChain(int startPos, int dirRow, int dirCol, PlayerId opponent) const {
    vector<int> captured;
    int r, c;
    positionToRowCol(startPos, r, c);

    int nr = r + dirRow;
    int nc = c + dirCol;

    while (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
        int npos = rowColToPosition(nr, nc);
        if (board[npos] == opponent) {
            captured.push_back(npos);
            nr += dirRow;
            nc += dirCol;
        }
        else {
            break;
        }
    }
    return captured;
}

bool Fanorona::hasCaptureFromPosition(int pos, PlayerId player, int forbiddenDirRow, int forbiddenDirCol,
    const vector<int>& visited) const {

    PlayerId opponent = (player == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
    int r, c;
    positionToRowCol(pos, r, c);

    for (int i = 0; i < 8; ++i) {
        int dr = DIRECTIONS[i][0];
        int dc = DIRECTIONS[i][1];

        bool hasForbiddenDir = (forbiddenDirRow != 0 || forbiddenDirCol != 0);
        if (hasForbiddenDir && dr == -forbiddenDirRow && dc == -forbiddenDirCol) continue;

        int nr = r + dr, nc = c + dc;
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;

        int destPos = rowColToPosition(nr, nc);
        if (board[destPos] != PlayerId::NONE) continue;
        if (!isConnected(pos, destPos)) continue;

        bool alreadyVisited = false;
        for (int v : visited) {
            if (v == destPos) { alreadyVisited = true; break; }
        }
        if (alreadyVisited) continue;

        vector<int> approachCaptures = getCaptureChain(destPos, dr, dc, opponent);
        if (!approachCaptures.empty()) return true;

        vector<int> withdrawalCaptures = getCaptureChain(pos, -dr, -dc, opponent);
        if (!withdrawalCaptures.empty()) return true;
    }
    return false;
}

bool Fanorona::hasAnyCaptureAvailable(PlayerId player) const {
    vector<int> emptyVisited;
    for (int pos = 0; pos < TOTAL_CELLS; ++pos) {
        if (board[pos] == player) {
            if (hasCaptureFromPosition(pos, player, 0, 0, emptyVisited)) return true;
        }
    }
    return false;
}

bool Fanorona::hasAnyLegalMove(PlayerId player) const {
    if (hasAnyCaptureAvailable(player)) return true;

    for (int pos = 0; pos < TOTAL_CELLS; ++pos) {
        if (board[pos] != player) continue;
        int r, c;
        positionToRowCol(pos, r, c);
        for (int i = 0; i < 8; ++i) {
            int nr = r + DIRECTIONS[i][0];
            int nc = c + DIRECTIONS[i][1];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            int destPos = rowColToPosition(nr, nc);
            if (board[destPos] == PlayerId::NONE && isConnected(pos, destPos)) return true;
        }
    }
    return false;
}

bool Fanorona::isValidMove(PlayerId player, const string& moveData) {
    if (isGameFinished || currentTurn != player) return false;

    stringstream ss(moveData);
    string typeStr;
    if (!getline(ss, typeStr, ',')) return false;
    if (typeStr.empty()) return false;
    char type = typeStr[0];

    if (type == 'E') {
        return waitingForChainCapture;
    }

    string fromStr, toStr;
    if (!getline(ss, fromStr, ',') || !getline(ss, toStr, ',')) return false;

    int from, to;
    try {
        from = stoi(fromStr);
        to = stoi(toStr);
    }
    catch (...) {
        return false;
    }

    if (from < 0 || from >= TOTAL_CELLS || to < 0 || to >= TOTAL_CELLS) return false;

    if (waitingForChainCapture) {
        if (from != chainPiecePos) return false;
        if (type != 'A' && type != 'W') return false;
    }
    else {
        if (type != 'P' && type != 'A' && type != 'W') return false;
    }

    if (board[from] != player) return false;
    if (board[to] != PlayerId::NONE) return false;
    if (!isConnected(from, to)) return false;

    int r1, c1, r2, c2;
    positionToRowCol(from, r1, c1);
    positionToRowCol(to, r2, c2);
    int dr = r2 - r1;
    int dc = c2 - c1;

    if (waitingForChainCapture) {
        if (dr == -lastDirRow && dc == -lastDirCol) return false;

        for (int v : visitedPositions) {
            if (v == to) return false;
        }
    }

    PlayerId opponent = (player == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;

    if (type == 'P') {
        if (hasAnyCaptureAvailable(player)) return false;
        return true;
    }
    else if (type == 'A') {
        vector<int> captures = getCaptureChain(to, dr, dc, opponent);
        return !captures.empty();
    }
    else { 
        vector<int> captures = getCaptureChain(from, -dr, -dc, opponent);
        return !captures.empty();
    }
}

bool Fanorona::applyMove(PlayerId player, const string& moveData) {
    if (!isValidMove(player, moveData)) return false;

    stringstream ss(moveData);
    string typeStr;
    getline(ss, typeStr, ',');
    char type = typeStr[0];

    if (type == 'E') {
        waitingForChainCapture = false;
        chainPiecePos = -1;
        visitedPositions.clear();
        checkGameOver();
        if (!isGameFinished) switchTurn();
        return true;
    }

    string fromStr, toStr;
    getline(ss, fromStr, ',');
    getline(ss, toStr, ',');
    int from = stoi(fromStr);
    int to = stoi(toStr);

    int r1, c1, r2, c2;
    positionToRowCol(from, r1, c1);
    positionToRowCol(to, r2, c2);
    int dr = r2 - r1;
    int dc = c2 - c1;

    PlayerId opponent = (player == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
    int oppIdx = static_cast<int>(opponent);

    board[to] = player;
    board[from] = PlayerId::NONE;

    if (type == 'P') {
        waitingForChainCapture = false;
        chainPiecePos = -1;
        visitedPositions.clear();
        checkGameOver();
        if (!isGameFinished) switchTurn();
        return true;
    }

    vector<int> captured;
    if (type == 'A') {
        captured = getCaptureChain(to, dr, dc, opponent);
    }
    else {
        captured = getCaptureChain(from, -dr, -dc, opponent);
    }

    for (int pos : captured) {
        board[pos] = PlayerId::NONE;
        pieceCount[oppIdx]--;
    }

    if (pieceCount[oppIdx] == 0) {
        isGameFinished = true;
        winnerPlayer = player;
        waitingForChainCapture = false;
        chainPiecePos = -1;
        visitedPositions.clear();
        return true;
    }

    if (visitedPositions.empty()) {
        visitedPositions.push_back(from);
    }
    visitedPositions.push_back(to);
    lastDirRow = dr;
    lastDirCol = dc;

    bool canContinue = hasCaptureFromPosition(to, player, dr, dc, visitedPositions);

    if (canContinue) {
        waitingForChainCapture = true;
        chainPiecePos = to;
    }
    else {
        waitingForChainCapture = false;
        chainPiecePos = -1;
        visitedPositions.clear();
        checkGameOver();
        if (!isGameFinished) switchTurn();
    }

    return true;
}

bool Fanorona::checkGameOver() {
    if (isGameFinished) return true;

    if (pieceCount[1] == 0) {
        isGameFinished = true;
        winnerPlayer = PlayerId::PLAYER_2;
        return true;
    }
    if (pieceCount[2] == 0) {
        isGameFinished = true;
        winnerPlayer = PlayerId::PLAYER_1;
        return true;
    }

    if (!waitingForChainCapture) {
        PlayerId nextPlayer = (currentTurn == PlayerId::PLAYER_1) ? PlayerId::PLAYER_2 : PlayerId::PLAYER_1;
        if (!hasAnyLegalMove(nextPlayer)) {
            isGameFinished = true;
            winnerPlayer = currentTurn; 
            return true;
        }
    }

    return false;
}

GameResult Fanorona::getResult() const {
    GameResult result;
    result.finished = isGameFinished;
    result.winner = winnerPlayer;
    result.isDraw = (isGameFinished && winnerPlayer == PlayerId::NONE);
    result.p1Score = pieceCount[1];
    result.p2Score = pieceCount[2];
    result.reason = isGameFinished ? "All pieces captured or opponent blocked" : "In progress";
    return result;
}

string Fanorona::getBoardStateString() const {
    stringstream ss;
    for (int i = 0; i < TOTAL_CELLS; ++i) {
        ss << static_cast<int>(board[i]);
    }
    return ss.str();
}

string Fanorona::serializeState() const {
    stringstream ss;
    ss << static_cast<int>(currentTurn) << ","
        << (waitingForChainCapture ? 1 : 0) << ","
        << chainPiecePos << ","
        << lastDirRow << ","
        << lastDirCol << ","
        << pieceCount[1] << ","
        << pieceCount[2] << ","
        << visitedPositions.size();

    for (int v : visitedPositions) {
        ss << "," << v;
    }

    for (int i = 0; i < TOTAL_CELLS; ++i) {
        ss << "," << static_cast<int>(board[i]);
    }

    return ss.str();
}

bool Fanorona::loadState(const string& stateData) {
    vector<string> tokens;
    stringstream ss(stateData);
    string token;

    while (getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 8) return false;

    try {
        currentTurn = static_cast<PlayerId>(stoi(tokens[0]));
        waitingForChainCapture = (stoi(tokens[1]) == 1);
        chainPiecePos = stoi(tokens[2]);
        lastDirRow = stoi(tokens[3]);
        lastDirCol = stoi(tokens[4]);
        pieceCount[1] = stoi(tokens[5]);
        pieceCount[2] = stoi(tokens[6]);

        int visitedCount = stoi(tokens[7]);
        visitedPositions.clear();

        int idx = 8;
        for (int i = 0; i < visitedCount; ++i) {
            if (idx >= static_cast<int>(tokens.size())) return false;
            visitedPositions.push_back(stoi(tokens[idx]));
            idx++;
        }

        if (idx + TOTAL_CELLS > static_cast<int>(tokens.size())) return false;
        for (int i = 0; i < TOTAL_CELLS; ++i) {
            board[i] = static_cast<PlayerId>(stoi(tokens[idx]));
            idx++;
        }

        return true;
    }
    catch (...) {
        return false;
    }
}