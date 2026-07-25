#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QMessageBox>
#include "mainmenu.h"
#include "networkmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRegularExpression phoneRegex("^09[0-9]{9}$");
    QRegularExpressionValidator *phoneValidator = new QRegularExpressionValidator(phoneRegex, this);
    ui->txt_phone_signup->setValidator(phoneValidator);
    ui->txt_phone_signup->setMaxLength(11);

    QRegularExpression emailRegex("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");
    QRegularExpressionValidator *emailValidator = new QRegularExpressionValidator(emailRegex, this);
    ui->txt_email_signup->setValidator(emailValidator);

    QFile styleFile(":/style.qss");
    if(styleFile.open(QFile::ReadOnly)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        setStyleSheet(styleSheet);
        styleFile.close();
    }

    ui->txt_password_Login->setEchoMode(QLineEdit::Password);
    ui->txt_password_signup->setEchoMode(QLineEdit::Password);
    ui->txt_new_password_forget->setEchoMode(QLineEdit::Password);

    ui->stackedWidget->setCurrentWidget(ui->page1_Login);

    connect(ui->btn_sign_up_Login, &QPushButton::clicked, this, &MainWindow::navigateToSignUp);
    connect(ui->btn_forget_password_Login, &QPushButton::clicked, this, &MainWindow::navigateToForgotPassword);
    connect(ui->btn_bazgasht_be_Login, &QPushButton::clicked, this, &MainWindow::navigateToLogin);
    connect(ui->btn_BackToLogin, &QPushButton::clicked, this, &MainWindow::navigateToLogin);

    connect(&NetworkManager::instance(), &NetworkManager::authResponseReceived, this, &MainWindow::onAuthResponseReceived);
    connect(&NetworkManager::instance(), &NetworkManager::connectionError, this, &MainWindow::onConnectionError);

    NetworkManager::instance().connectToServer("127.0.0.1", 12345);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::navigateToSignUp()
{
    ui->txt_name_signup->clear();
    ui->txt_username_signup->clear();
    ui->txt_phone_signup->clear();
    ui->txt_email_signup->clear();
    ui->txt_password_signup->clear();

    ui->stackedWidget->setCurrentWidget(ui->page_2_sign_up);
}

void MainWindow::navigateToForgotPassword()
{
    ui->txt_phone_number_forget->clear();
    ui->txt_new_password_forget->clear();
    ui->txt_username_forget->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_3_forgot_password);
}

void MainWindow::navigateToLogin()
{
    ui->txt_username_Login->clear();
    ui->txt_password_Login->clear();

    ui->stackedWidget->setCurrentWidget(ui->page1_Login);
}

void MainWindow::setFieldErrorStyle(QLineEdit *widget, bool isError) {
    if (isError) {
        widget->setStyleSheet("border: 2px solid red; border-radius: 15px;");
    } else {
        widget->setStyleSheet("");
    }
}

void MainWindow::on_btn_sign_up_signup_clicked() {
    bool hasError = false;

    if (ui->txt_name_signup->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_name_signup, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_name_signup, false); }

    if (ui->txt_username_signup->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_username_signup, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_username_signup, false); }

    if (ui->txt_password_signup->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_password_signup, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_password_signup, false); }

    if (ui->txt_phone_signup->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_phone_signup, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_phone_signup, false); }

    if (ui->txt_email_signup->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_email_signup, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_email_signup, false); }

    if (hasError) {
        QMessageBox::warning(this, "Error", "Please fill in all fields.");
        return;
    }

    QString name = ui->txt_name_signup->text();
    QString username = ui->txt_username_signup->text();
    QString pass = ui->txt_password_signup->text();
    QString phone = ui->txt_phone_signup->text();
    QString email = ui->txt_email_signup->text();

    lastAttemptedUsername = username;

    QString payload = "REGISTER|" + name + "|" + username + "|" + pass + "|" + phone + "|" + email;
    NetworkManager::instance().sendPacket(PacketType::CONNECT_REQ, "Guest", payload);
}

void MainWindow::on_btn_Login_Login_clicked() {
    QString username = ui->txt_username_Login->text();
    QString password = ui->txt_password_Login->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password.");
        if(username.isEmpty()) setFieldErrorStyle(ui->txt_username_Login, true);
        if(password.isEmpty()) setFieldErrorStyle(ui->txt_password_Login, true);
        return;
    }

    lastAttemptedUsername = username;

    QString payload = "LOGIN|" + username + "|" + password;
    NetworkManager::instance().sendPacket(PacketType::CONNECT_REQ, "Guest", payload);
}

void MainWindow::on_btn_login_forget_clicked() {
    QString username = ui->txt_username_forget->text();
    QString phone = ui->txt_phone_number_forget->text();
    QString newPass = ui->txt_new_password_forget->text();

    bool hasError = false;
    if (username.isEmpty()) { setFieldErrorStyle(ui->txt_username_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_username_forget, false); }

    if (phone.isEmpty()) { setFieldErrorStyle(ui->txt_phone_number_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_phone_number_forget, false); }

    if (newPass.isEmpty()) { setFieldErrorStyle(ui->txt_new_password_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_new_password_forget, false); }

    if (hasError) {
         QMessageBox::warning(this, "Error", "Please fill in all fields.");
         return;
    }

    lastAttemptedUsername = username;

    QString payload = "FORGOT_PASS|" + username + "|" + phone + "|" + newPass;
    NetworkManager::instance().sendPacket(PacketType::CONNECT_REQ, "Guest", payload);
}

void MainWindow::onAuthResponseReceived(bool isSuccess, QString message) {
    if (isSuccess) {

        // قطع اتصال سیگنال‌های این فرم برای جلوگیری از کرش کردن و مزاحمت در پس‌زمینه
        disconnect(&NetworkManager::instance(), &NetworkManager::authResponseReceived, this, &MainWindow::onAuthResponseReceived);
        disconnect(&NetworkManager::instance(), &NetworkManager::connectionError, this, &MainWindow::onConnectionError);

        QStringList parts = message.split('|');
        User tempUser("", lastAttemptedUsername.toStdString(), "", "", "");

        if (parts.size() >= 5 && parts[0] == "LOGIN_SUCCESS") {
            int dotsScore = parts[1].toInt();
            int nineScore = parts[2].toInt();
            int fanScore = parts[3].toInt();

            tempUser.updateScore(GameType::DotsAndBoxes, dotsScore);
            tempUser.updateScore(GameType::NineMensMorris, nineScore);
            tempUser.updateScore(GameType::Fanorona, fanScore);

            int historyCount = parts[4].toInt();
            int index = 5;

            for (int i = 0; i < historyCount; ++i) {
                if (index + 5 < parts.size()) {
                    GameRecord record;
                    record.gameName = static_cast<GameType>(parts[index++].toInt());
                    record.opponent = parts[index++].toStdString();
                    record.date = parts[index++].toStdString();
                    record.playerRole = parts[index++].toStdString();
                    record.result = parts[index++].toStdString();
                    record.score = parts[index++].toInt();
                    tempUser.addGameRecord(record);
                }
            }
        }

        QMessageBox::information(this, "Success", "Login Successful!");
        MainMenu *mainMenuWindow = new MainMenu(tempUser);
        mainMenuWindow->show();
        this->close();
    } else {
        QMessageBox::warning(this, "Error", message);
    }
}

void MainWindow::onConnectionError(QString errorMsg) {
    QMessageBox::critical(this, "Connection Error", errorMsg);
}
