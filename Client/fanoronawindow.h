#ifndef FANORONAWINDOW_H
#define FANORONAWINDOW_H

#include <QMainWindow>
#include <QNetworkInterface>
#include <QTimer>
#include <QProcess>
#include <vector>
#include "User.h"

namespace Ui {
class FanoronaWindow;
}

class FanoronaWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FanoronaWindow(const User& user, QWidget *parent = nullptr);
    ~FanoronaWindow();

private slots:
    void on_btn_start_new_game_clicked();
    void on_btn_back_clicked();
    void on_btn_back_to_dashboard_clicked();
    void on_chk_time_limit_stateChanged(int arg1);
    void on_btn_create_room_clicked();
    void on_btn_join_room_clicked();
    void on_btn_select_host_clicked();
    void on_btn_select_guest_clicked();

    void onPositionClicked(int posIndex);
    void on_btn_end_chain_turn_clicked();

    void onRoomJoined(QString message);
    void onGameStarted(QString message);
    void onErrorReceived(QString errorMsg);
    void onConnectionError(QString errorMsg);
    void onMoveReceived(QString moveData);
    void onTurnChangedReceived();
    void onTurnTimerTick();
    void onGameOverReceived(QString message);
    void onServerConnected();

private:
    Ui::FanoronaWindow *ui;
    User currentUser;

    QTimer *m_turnTimer;
    QProcess *m_localServerProcess;
    bool m_pendingCreateRoom;
    bool m_pendingJoinRoom;
    bool m_isGameOver;

    int m_timeLimit;
    int m_remainingTime;
    bool m_isMyTurn;
    int m_myPlayerId;
    QString m_opponentUsername;

    static const int ROWS = 5;
    static const int COLS = 9;
    static const int TOTAL_CELLS = 45;
    static const int DIRECTIONS[8][2];

    std::vector<int> m_board;
    int m_p1Count;
    int m_p2Count;

    bool m_waitingForChainCapture;
    int m_chainPiecePos;
    int m_lastDirRow;
    int m_lastDirCol;
    std::vector<int> m_visitedPositions;
    int m_selectedPos;

    void displayLocalIP();
    void setupDashboardUI();
    void setupNetworkUI();
    void setHostMode(bool isHost);
    void cleanupNetworkAndServer();

    void initGame(bool isHost, QString opponent, int timeLimit);
    void positionToRowCol(int pos, int& r, int& c) const;
    int rowColToPosition(int r, int c) const;
    bool isConnected(int from, int to) const;
    std::vector<int> getCaptureChain(int startPos, int dirRow, int dirCol, int opponent) const;
    bool hasCaptureFromPosition(int pos, int player, int forbiddenDirRow, int forbiddenDirCol, const std::vector<int>& visited) const;
    bool hasAnyLegalMove(int player) const;
    bool hasAnyCaptureAvailable(int player) const;
    std::vector<int> getValidDestinations(int pos) const;

    void applyLocalMove(int player, char type, int from, int to);
    void checkGameOver();
    void updateGameUI();
    void endTurn();
};

#endif
