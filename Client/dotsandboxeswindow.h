#ifndef DOTSANDBOXESWINDOW_H
#define DOTSANDBOXESWINDOW_H

#include <QWidget>
#include "User.h"

namespace Ui {
class DotsAndBoxesWindow;
}

class DotsAndBoxesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DotsAndBoxesWindow(const User& user, QWidget *parent = nullptr);
    ~DotsAndBoxesWindow();

private slots:
    void on_btn_back_clicked();
    void on_btn_start_game_clicked();
    void on_btn_back_to_dashboard_clicked();

    void onLineClicked(int row, int col, bool isHorizontal);
private:
    Ui::DotsAndBoxesWindow *ui;
    User currentUser;

    void loadGameHistory();
};

#endif
