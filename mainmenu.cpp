#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "mainwindow.h"
#include <QMessageBox>

MainMenu::MainMenu(const User& user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenu),
    currentUser(user)
{
    ui->setupUi(this);

    userManager.loadFromFile("users.txt");

    ui->mainmenu->setCurrentWidget(ui->Page1_mainmenu);

    ui->txt_password_edit->setEchoMode(QLineEdit::Password);

    ui->lbl_welcome->setText("Welcome " + QString::fromStdString(currentUser.getName()) + " 👋");
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::setFieldErrorStyle(QLineEdit *widget, bool isError) {
    if (isError) {
        widget->setStyleSheet("border: 2px solid red; border-radius: 10px;");
    } else {
        widget->setStyleSheet("");
    }
}

void MainMenu::on_btn_logout_clicked()
{
    MainWindow *loginWindow = new MainWindow();
    loginWindow->show();
    this->close();
}

void MainMenu::on_btn_edit_profile_clicked()
{
    ui->txt_name_edit->setText(QString::fromStdString(currentUser.getName()));
    ui->txt_username_edit->setText(QString::fromStdString(currentUser.getUsername()));
    ui->txt_phone_edit->setText(QString::fromStdString(currentUser.getPhoneNumber()));
    ui->txt_email_edit->setText(QString::fromStdString(currentUser.getEmail()));
    ui->txt_password_edit->clear();

    setFieldErrorStyle(ui->txt_name_edit, false);
    setFieldErrorStyle(ui->txt_username_edit, false);
    setFieldErrorStyle(ui->txt_password_edit, false);
    setFieldErrorStyle(ui->txt_phone_edit, false);
    setFieldErrorStyle(ui->txt_email_edit, false);

    ui->mainmenu->setCurrentWidget(ui->page_2_edit_profile);
}

void MainMenu::on_btn_back_to_main_clicked()
{
    ui->mainmenu->setCurrentWidget(ui->Page1_mainmenu);
}

void MainMenu::on_btn_save_profile_clicked()
{
    bool hasError = false;

    if (ui->txt_name_edit->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_name_edit, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_name_edit, false); }

    if (ui->txt_username_edit->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_username_edit, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_username_edit, false); }

    if (ui->txt_password_edit->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_password_edit, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_password_edit, false); }

    if (ui->txt_phone_edit->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_phone_edit, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_phone_edit, false); }

    if (ui->txt_email_edit->text().isEmpty()) {
        setFieldErrorStyle(ui->txt_email_edit, true);
        hasError = true;
    } else { setFieldErrorStyle(ui->txt_email_edit, false); }

    if (hasError) {
        QMessageBox::warning(this, "اخطار", "لطفاً تمام فیلدها را پر کنید.");
        return;
    }

    std::string newName = ui->txt_name_edit->text().toStdString();
    std::string newUsername = ui->txt_username_edit->text().toStdString();
    std::string newPass = ui->txt_password_edit->text().toStdString();
    std::string newPhone = ui->txt_phone_edit->text().toStdString();
    std::string newEmail = ui->txt_email_edit->text().toStdString();

    AuthStatus status = userManager.updateUserProfile(
        currentUser.getUsername(),
        newName,
        newUsername,
        newPass,
        newPhone,
        newEmail
    );

    switch (status) {
        case AuthStatus::Success:
            QMessageBox::information(this, "موفقیت", "اطلاعات حساب کاربری با موفقیت به روزرسانی شد.");

            userManager.saveToFile("users.txt");

            if (const User* updatedUser = userManager.getUser(newUsername)) {
                currentUser = *updatedUser;
            }

            ui->lbl_welcome->setText("Welcome " + QString::fromStdString(currentUser.getName()) + " 👋");
            ui->mainmenu->setCurrentWidget(ui->Page1_mainmenu);
            break;

        case AuthStatus::UsernameTaken:
            QMessageBox::warning(this, "خطا", "این نام کاربری قبلاً توسط شخص دیگری انتخاب شده است.");
            setFieldErrorStyle(ui->txt_username_edit, true);
            break;

        case AuthStatus::PasswordTooShort:
            QMessageBox::warning(this, "خطا", "رمز عبور جدید باید حداقل ۸ کاراکتر باشد.");
            setFieldErrorStyle(ui->txt_password_edit, true);
            break;

        case AuthStatus::InvalidPhone:
            QMessageBox::warning(this, "خطا", "فرمت شماره تلفن وارد شده صحیح نیست.");
            setFieldErrorStyle(ui->txt_phone_edit, true);
            break;

        case AuthStatus::InvalidEmail:
            QMessageBox::warning(this, "خطا", "فرمت ایمیل وارد شده صحیح نیست.");
            setFieldErrorStyle(ui->txt_email_edit, true);
            break;

        default:
            QMessageBox::critical(this, "خطا", "خطای غیرمنتظره‌ای در به‌روزرسانی رخ داد.");
            break;
    }
}
