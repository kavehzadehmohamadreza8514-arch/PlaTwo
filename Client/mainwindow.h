#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include "UserManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void navigateToSignUp();
    void navigateToForgotPassword();
    void navigateToLogin();

    void setFieldErrorStyle(QLineEdit *widget, bool isError);

    void on_btn_sign_up_signup_clicked();
    void on_btn_Login_Login_clicked();
    void on_btn_login_forget_clicked();
private:
    Ui::MainWindow *ui;
    UserManager userManager;
};
#endif
