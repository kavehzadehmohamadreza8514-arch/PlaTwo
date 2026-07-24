#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QLineEdit>
#include "User.h"

namespace Ui {
class MainMenu;
}

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(const User& currentUser, QWidget *parent = nullptr);
    ~MainMenu();

private slots:
    void on_btn_logout_clicked();
    void on_btn_edit_profile_clicked();
    void on_btn_back_to_main_clicked();
    void on_btn_save_profile_clicked();
    void on_btn_boxes_and_dots_clicked();

private:
    Ui::MainMenu *ui;
    User currentUser;

    void setFieldErrorStyle(QLineEdit *widget, bool isError);
};

#endif
