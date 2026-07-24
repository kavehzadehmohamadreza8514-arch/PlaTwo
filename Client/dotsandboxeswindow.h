#ifndef DOTSANDBOXESWINDOW_H
#define DOTSANDBOXESWINDOW_H

#include <QMainWindow>
#include <QNetworkInterface>
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

    void on_chk_time_limit_dots_and_boxes_stateChanged(int arg1);
    void on_btn_create_room_dots_and_boxes_clicked();
    void on_btn_join_room_ip_dots_and_boxes_clicked();
    void onLineClicked(int row, int col, bool isHorizontal);

    void onRoomJoined(QString message);
    void onGameStarted(QString message);
    void onErrorReceived(QString errorMsg);

    void on_btn_select_host_clicked();
    void on_btn_select_guest_clicked();

private:
    Ui::DotsAndBoxesWindow *ui;
    User currentUser;

    void displayLocalIP();
    void setupDashboardUI();
    void setupNetworkUI();
    void setHostMode(bool isHost);
};

#endif
