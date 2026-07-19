#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    userManager.loadFromFile("users.txt");

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

    // ۱. بررسی خالی بودن فیلدها و قرمز کردن حاشیه در صورت نیاز
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
        QMessageBox::warning(this, "اخطار", "لطفاً تمام فیلدها را پر کنید.");
        return;
    }

    // ۲. استخراج داده‌ها
    std::string name = ui->txt_name_signup->text().toStdString();
    std::string username = ui->txt_username_signup->text().toStdString();
    std::string pass = ui->txt_password_signup->text().toStdString();
    std::string phone = ui->txt_phone_signup->text().toStdString();
    std::string email = ui->txt_email_signup->text().toStdString();

    // ۳. ارسال به هسته منطقی
    AuthStatus status = userManager.registerUser(name, username, pass, phone, email);

    // ۴. مدیریت پاسخ
    switch (status) {
        case AuthStatus::Success:
            QMessageBox::information(this, "موفقیت", "ثبت‌نام با موفقیت انجام شد.");
            userManager.saveToFile("users.txt");
            navigateToLogin();
            break;
        case AuthStatus::UsernameTaken:
            QMessageBox::warning(this, "خطا", "این نام کاربری قبلاً انتخاب شده است.");
            setFieldErrorStyle(ui->txt_username_signup, true);
            break;
        case AuthStatus::PasswordTooShort:
            QMessageBox::warning(this, "خطا", "رمز عبور باید حداقل ۸ کاراکتر باشد.");
            setFieldErrorStyle(ui->txt_password_signup, true);
            break;
        case AuthStatus::InvalidPhone:
            QMessageBox::warning(this, "خطا", "فرمت شماره تلفن صحیح نیست.");
            setFieldErrorStyle(ui->txt_phone_signup, true);
            break;
        case AuthStatus::InvalidEmail:
            QMessageBox::warning(this, "خطا", "فرمت ایمیل صحیح نیست.");
            setFieldErrorStyle(ui->txt_email_signup, true);
            break;
        default:
            QMessageBox::critical(this, "خطا", "خطای سیستمی رخ داده است.");
            break;
    }
}

void MainWindow::on_btn_Login_Login_clicked() {
    std::string username = ui->txt_username_Login->text().toStdString();
    std::string password = ui->txt_password_Login->text().toStdString();

    if (username.empty() || password.empty()) {
        QMessageBox::warning(this, "اخطار", "لطفاً نام کاربری و رمز عبور را وارد کنید.");
        if(username.empty()) setFieldErrorStyle(ui->txt_username_Login, true);
        if(password.empty()) setFieldErrorStyle(ui->txt_password_Login, true);
        return;
    }

    AuthStatus status = userManager.loginUser(username, password);

    if (status == AuthStatus::Success) {
        setFieldErrorStyle(ui->txt_username_Login, false);
        setFieldErrorStyle(ui->txt_password_Login, false);
        QMessageBox::information(this, "موفقیت", "ورود با موفقیت انجام شد.");
        // کد انتقال به صفحه داشبورد در آینده اینجا قرار می‌گیرد
    }
    else if (status == AuthStatus::IncorrectPassword) {
        setFieldErrorStyle(ui->txt_password_Login, true);
        QMessageBox::warning(this, "خطا", "رمز عبور اشتباه است.");
    }
    else if (status == AuthStatus::UsernameNotFound) {
        setFieldErrorStyle(ui->txt_username_Login, true);
        QMessageBox::warning(this, "خطا", "کاربری با این نام یافت نشد.");
    }
}

void MainWindow::on_btn_login_forget_clicked() {
    std::string username = ui->txt_username_forget->text().toStdString();
    std::string phone = ui->txt_phone_number_forget->text().toStdString();
    std::string newPass = ui->txt_new_password_forget->text().toStdString();

    bool hasError = false;
    if (username.empty()) { setFieldErrorStyle(ui->txt_username_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_username_forget, false); }

    if (phone.empty()) { setFieldErrorStyle(ui->txt_phone_number_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_phone_number_forget, false); }

    if (newPass.empty()) { setFieldErrorStyle(ui->txt_new_password_forget, true); hasError = true; }
    else { setFieldErrorStyle(ui->txt_new_password_forget, false); }

    if (hasError) {
         QMessageBox::warning(this, "اخطار", "لطفاً تمام فیلدها را پر کنید.");
         return;
    }

    AuthStatus status = userManager.resetPasswordWithPhone(username, phone, newPass);

    if (status == AuthStatus::Success) {
        QMessageBox::information(this, "موفقیت", "رمز عبور با موفقیت تغییر کرد.");
        userManager.saveToFile("users.txt");
        navigateToLogin();
    }
    else if (status == AuthStatus::PhoneMismatch) {
        setFieldErrorStyle(ui->txt_phone_number_forget, true);
        QMessageBox::warning(this, "خطا", "شماره تلفن با نام کاربری همخوانی ندارد.");
    }
    else if (status == AuthStatus::UsernameNotFound) {
        setFieldErrorStyle(ui->txt_username_forget, true);
        QMessageBox::warning(this, "خطا", "نام کاربری یافت نشد.");
    }
    else if (status == AuthStatus::PasswordTooShort) {
        setFieldErrorStyle(ui->txt_new_password_forget, true);
        QMessageBox::warning(this, "خطا", "رمز عبور جدید باید حداقل ۸ کاراکتر باشد.");
    }
}












