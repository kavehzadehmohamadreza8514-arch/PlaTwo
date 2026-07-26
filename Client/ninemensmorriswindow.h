#ifndef NINEMENSMORRISWINDOW_H
#define NINEMENSMORRISWINDOW_H

#include <QMainWindow>
#include <QNetworkInterface>
#include <QTimer>
#include <QProcess>
#include <vector>
#include "User.h"

namespace Ui {
class NineMensMorrisWindow;
}

class NineMensMorrisWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NineMensMorrisWindow(const User& user, QWidget *parent = nullptr);
    ~NineMensMorrisWindow();

private slots:
    void on_btn_start_new_game_clicked();
    void on_btn_back_clicked();
    void on_btn_back_to_dashboard_clicked();

    void on_chk_time_limit_stateChanged(int arg1);
    void on_btn_create_room_clicked();
    void on_btn_join_room_clicked();
    void onPositionClicked(int posIndex);

    void onRoomJoined(QString message);
    void onGameStarted(QString message);
    void onErrorReceived(QString errorMsg);
    void onConnectionError(QString errorMsg);

    void on_btn_select_host_clicked();
    void on_btn_select_guest_clicked();

    void onMoveReceived(QString moveData);
    void onTurnChangedReceived();
    void onTurnTimerTick();
    void onGameOverReceived(QString message);

    void onServerConnected();

private:
    Ui::NineMensMorrisWindow *ui;
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

    std::vector<int> m_board;
    int m_p1Unplaced;
    int m_p2Unplaced;
    int m_p1Count;
    int m_p2Count;
    bool m_isRemovingState;
    int m_selectedPos;

    void displayLocalIP();
    void setupDashboardUI();
    void setupNetworkUI();
    void setHostMode(bool isHost);

    void initGame(bool isHost, QString opponent, int timeLimit);
    bool checkMill(int pos, int player);
    bool isAdjacent(int from, int to);
    bool hasLegalMoves(int player);
    void checkGameOver();
    void updateGameUI();
    void endTurn();
    void cleanupNetworkAndServer();
};

#endif
