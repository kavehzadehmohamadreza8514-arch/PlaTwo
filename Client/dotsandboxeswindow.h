#ifndef DOTSANDBOXESWINDOW_H
#define DOTSANDBOXESWINDOW_H

#include <QMainWindow>
#include <QNetworkInterface>
#include <QTimer>
#include <QProcess>
#include <QComboBox>
#include <vector>
#include "User.h"

namespace Ui {
class DotsAndBoxesWindow;
}

class DotsAndBoxesWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DotsAndBoxesWindow(const User& user, QWidget *parent = nullptr);
    ~DotsAndBoxesWindow();

private slots:
    void on_btn_start_new_game_clicked();
    void on_btn_back_clicked();
    void on_btn_back_to_dashboard_clicked();

    void on_chk_time_limit_dots_and_boxes_stateChanged(int arg1);
    void on_btn_create_room_dots_and_boxes_clicked();
    void on_btn_join_room_ip_dots_and_boxes_clicked();
    void onLineClicked(int row, int col, bool isHorizontal);

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
    Ui::DotsAndBoxesWindow *ui;
    User currentUser;

    QTimer *m_turnTimer;
    QProcess *m_localServerProcess;
    bool m_pendingCreateRoom;
    bool m_pendingJoinRoom;
    bool m_isGameOver;

    int m_timeLimit;
    int m_remainingTime;
    int m_boardSize;
    bool m_isMyTurn;
    int m_myPlayerId;
    int m_p1Score;
    int m_p2Score;
    QString m_opponentUsername;

    QComboBox* combo_host_color;
    QComboBox* combo_guest_color;

    std::vector<std::vector<int>> m_hLines;
    std::vector<std::vector<int>> m_vLines;
    std::vector<std::vector<int>> m_boxes;

    void displayLocalIP();
    void setupDashboardUI();
    void setupNetworkUI();
    void setHostMode(bool isHost);

    void initGame(int boardSize, int timeLimit, bool isHost, QString opponent, QString hostColorStr, QString guestColorStr);
    int checkAndClaimBoxes(int r, int c, bool isHorizontal, int playerId);
    void checkGameOver();
    void updateGameUI();
    void endTurn();
    void cleanupNetworkAndServer();

    QColor getColorFromString(const QString& colorName);
};

#endif
