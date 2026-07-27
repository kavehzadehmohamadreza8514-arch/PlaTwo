#include "mainmenu.h"
#include "ui_mainmenu.h"
#include "mainwindow.h"
#include <QMessageBox>
#include "dotsandboxeswindow.h"
#include "networkmanager.h"
#include "ninemensmorriswindow.h"

MainMenu::MainMenu(const User& user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMenu),
    currentUser(user)
{
    ui->setupUi(this);

    ui->mainmenu->setCurrentWidget(ui->Page1_mainmenu);

    ui->txt_password_edit->setEchoMode(QLineEdit::Password);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->lbl_welcome->setText("Welcome " + QString::fromStdString(currentUser.getUsername()));

    connect(&NetworkManager::instance(), &NetworkManager::authResponseReceived, this, &MainMenu::onUpdateProfileResponse);
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
        QMessageBox::warning(this, "Error", "Please fill in all fields.");
        return;
    }

    QString currentUsername = QString::fromStdString(currentUser.getUsername());
    QString newName = ui->txt_name_edit->text();
    QString newUsername = ui->txt_username_edit->text();
    QString newPass = ui->txt_password_edit->text();
    QString newPhone = ui->txt_phone_edit->text();
    QString newEmail = ui->txt_email_edit->text();

    QString payload = "UPDATE_PROFILE|" + currentUsername + "|" + newName + "|" + newUsername + "|" + newPass + "|" + newPhone + "|" + newEmail;
    NetworkManager::instance().sendPacket(PacketType::CONNECT_REQ, currentUsername, payload);

    ui->btn_save_profile->setText("Saving...");
    ui->btn_save_profile->setEnabled(false);
}

void MainMenu::onUpdateProfileResponse(bool isSuccess, QString message)
{
    ui->btn_save_profile->setText("Save Changes");
    ui->btn_save_profile->setEnabled(true);

    if (isSuccess && message == "UPDATE_SUCCESS") {
        QMessageBox::information(this, "Success", "Profile updated successfully!");

        currentUser.setName(ui->txt_name_edit->text().toStdString());
        currentUser.setUsername(ui->txt_username_edit->text().toStdString());
        currentUser.setPhoneNumber(ui->txt_phone_edit->text().toStdString());
        currentUser.setEmail(ui->txt_email_edit->text().toStdString());

        ui->lbl_welcome->setText("Welcome " + QString::fromStdString(currentUser.getName()));
        ui->mainmenu->setCurrentWidget(ui->Page1_mainmenu);
    } else {
        if (message.contains("Username") || message.contains("Failed") || message.contains("Profile") || message.contains("short") || message.contains("Invalid")) {
            QMessageBox::warning(this, "Update Failed", message);
        }
    }
}

void MainMenu::on_btn_boxes_and_dots_clicked()
{
    DotsAndBoxesWindow *dotsWindow = new DotsAndBoxesWindow(currentUser, this);
    dotsWindow->setAttribute(Qt::WA_DeleteOnClose);
    dotsWindow->show();

    this->hide();
}

void MainMenu::on_btn_nine_mens_morris_clicked()
{
    NineMensMorrisWindow *nineMensWindow = new NineMensMorrisWindow(currentUser, this);
    nineMensWindow->setAttribute(Qt::WA_DeleteOnClose);
    nineMensWindow->show();

    this->hide();
}

void MainMenu::updateUserData(const User& updatedUser)
{
    this->currentUser = updatedUser;
    ui->lbl_welcome->setText("Welcome " + QString::fromStdString(currentUser.getName()));
}
