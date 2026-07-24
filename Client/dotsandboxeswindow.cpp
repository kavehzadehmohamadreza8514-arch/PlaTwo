#include "dotsandboxeswindow.h"
#include "ui_dotsandboxeswindow.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include "networkmanager.h"

DotsAndBoxesWindow::DotsAndBoxesWindow(const User& user, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DotsAndBoxesWindow),
    currentUser(user)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);

    ui->spin_time_min_dots_and_boxes->setEnabled(false);
    ui->spin_time_sec_dots_and_boxes->setEnabled(false);

    ui->lbl_score->setText(QString::number(currentUser.getDotsAndBoxesScore()));

    setupDashboardUI();
    setupNetworkUI();

    connect(ui->board_widget, &GameBoardWidget::lineClicked, this, &DotsAndBoxesWindow::onLineClicked);

    connect(&NetworkManager::instance(), &NetworkManager::roomJoined, this, &DotsAndBoxesWindow::onRoomJoined);
    connect(&NetworkManager::instance(), &NetworkManager::gameStarted, this, &DotsAndBoxesWindow::onGameStarted);
    connect(&NetworkManager::instance(), &NetworkManager::errorReceived, this, &DotsAndBoxesWindow::onErrorReceived);
}

DotsAndBoxesWindow::~DotsAndBoxesWindow()
{
    delete ui;
}

void DotsAndBoxesWindow::setupDashboardUI()
{
    ui->frame_score->setStyleSheet(
        "QFrame#frame_score {"
        "   background-color: rgba(20, 30, 48, 0.45);"
        "   border: 2px solid #00f0b5;"
        "   border-radius: 20px;"
        "}"
    );

    ui->frame_log->setStyleSheet(
        "QFrame#frame_log {"
        "   background-color: rgba(20, 30, 48, 0.40);"
        "   border: 1px solid rgba(20, 202, 214, 0.3);"
        "   border-radius: 25px;"
        "}"
    );

    ui->tbl_history->setColumnCount(5);
    QStringList headers = {"Opponent", "Date", "Role", "Result", "Score"};
    ui->tbl_history->setHorizontalHeaderLabels(headers);

    ui->tbl_history->setStyleSheet(
        "QTableWidget {"
        "   background-color: transparent;"
        "   gridline-color: transparent;"
        "   border: none;"
        "   color: white;"
        "   font-size: 14px;"
        "}"
        "QHeaderView::section {"
        "   background-color: transparent;"
        "   color: #a2a8d3;"
        "   border: none;"
        "   font-weight: bold;"
        "   padding: 5px;"
        "}"
        "QTableWidget::item {"
        "   background-color: rgba(255, 255, 255, 0.08);"
        "   border-radius: 12px;"
        "   margin: 4px;"
        "   padding: 5px;"
        "}"
    );

    ui->tbl_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tbl_history->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tbl_history->setShowGrid(false);
    ui->tbl_history->setRowCount(0);

    const std::vector<GameRecord>& history = currentUser.getGameHistory();

    for (const auto& record : history) {
        if (record.gameName == GameType::DotsAndBoxes) {
            int row = ui->tbl_history->rowCount();
            ui->tbl_history->insertRow(row);

            ui->tbl_history->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.opponent)));
            ui->tbl_history->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.date)));
            ui->tbl_history->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.playerRole)));

            QTableWidgetItem *resultItem = new QTableWidgetItem(QString::fromStdString(record.result));
            if (record.result == "Win" || record.result == "WIN") {
                resultItem->setForeground(QColor("#00ffcc"));
            } else {
                resultItem->setForeground(QColor("#ff4d6d"));
            }
            ui->tbl_history->setItem(row, 3, resultItem);

            ui->tbl_history->setItem(row, 4, new QTableWidgetItem(QString::number(record.score)));

            for (int col = 0; col < 5; ++col) {
                if (ui->tbl_history->item(row, col)) {
                    ui->tbl_history->item(row, col)->setTextAlignment(Qt::AlignCenter);
                }
            }
        }
    }
}

void DotsAndBoxesWindow::setupNetworkUI()
{
    ui->txt_host_port_dots_and_boxes->setPlaceholderText("ENTER HOST PORT (default 8080)");
    ui->txt_guest_ip_dots_and_boxes->setPlaceholderText("ENTER HOST USERNAME (Room ID)");
    ui->txt_guest_port_ip_dots_and_boxes->setPlaceholderText("ENTER HOST PORT (default 8080)");

    QString baseFrameStyle =
        "QFrame {"
        "   background-color: rgba(30, 40, 60, 0.4);"
        "   border-radius: 25px;"
        "}";

    QString lineEditStyle =
        "QLineEdit {"
        "   background-color: rgba(0, 0, 0, 0.3);"
        "   border: 1px solid rgba(255, 255, 255, 0.2);"
        "   border-radius: 15px;"
        "   color: white;"
        "   padding: 10px 15px;"
        "}";

    ui->frame_mode_selector->setStyleSheet(baseFrameStyle);
    ui->frame_host_setup->setStyleSheet(baseFrameStyle);
    ui->frame_guest_setup->setStyleSheet(baseFrameStyle);

    ui->txt_host_port_dots_and_boxes->setStyleSheet(lineEditStyle);
    ui->txt_guest_ip_dots_and_boxes->setStyleSheet(lineEditStyle);
    ui->txt_guest_port_ip_dots_and_boxes->setStyleSheet(lineEditStyle);

    ui->combo_board_size_dots_and_boxes->setStyleSheet(
        "QComboBox {"
        "   background-color: rgba(0, 0, 0, 0.3);"
        "   border: 1px solid #00f0b5;"
        "   border-radius: 15px;"
        "   color: #00f0b5;"
        "   padding: 5px 15px;"
        "}"
    );

    ui->lbl_host_ip_dots_and_boxes->setStyleSheet(
        "QLabel {"
        "   border: 1px solid #00f0b5;"
        "   border-radius: 15px;"
        "   color: #00f0b5;"
        "   padding: 5px;"
        "}"
    );

    connect(ui->btn_select_host, &QPushButton::clicked, this, &DotsAndBoxesWindow::on_btn_select_host_clicked);
    connect(ui->btn_select_guest, &QPushButton::clicked, this, &DotsAndBoxesWindow::on_btn_select_guest_clicked);

    setHostMode(true);
}

void DotsAndBoxesWindow::setHostMode(bool isHost)
{
    QString activeHostStyle =
        "QFrame#frame_host_setup {"
        "   background-color: rgba(20, 30, 48, 0.6);"
        "   border: 2px solid #00f0b5;"
        "   border-radius: 25px;"
        "}";

    QString inactiveStyle =
        "QFrame#frame_host_setup, QFrame#frame_guest_setup {"
        "   background-color: rgba(20, 30, 48, 0.2);"
        "   border: 2px solid rgba(255, 255, 255, 0.1);"
        "   border-radius: 25px;"
        "}";

    QString activeGuestStyle =
        "QFrame#frame_guest_setup {"
        "   background-color: rgba(20, 30, 48, 0.6);"
        "   border: 2px solid #ffde59;"
        "   border-radius: 25px;"
        "}";

    if (isHost) {
        ui->frame_host_setup->setStyleSheet(activeHostStyle);
        ui->frame_guest_setup->setStyleSheet(inactiveStyle);
        ui->frame_host_setup->setEnabled(true);
        ui->frame_guest_setup->setEnabled(false);

        ui->btn_select_host->setStyleSheet("QPushButton { color: #00f0b5; font-weight: bold; background: transparent; border: none; }");
        ui->btn_select_guest->setStyleSheet("QPushButton { color: gray; background: transparent; border: none; }");

        ui->btn_create_room_dots_and_boxes->setStyleSheet(
            "QPushButton { background-color: #00f0b5; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }"
        );
    } else {
        ui->frame_host_setup->setStyleSheet(inactiveStyle);
        ui->frame_guest_setup->setStyleSheet(activeGuestStyle);
        ui->frame_host_setup->setEnabled(false);
        ui->frame_guest_setup->setEnabled(true);

        ui->btn_select_host->setStyleSheet("QPushButton { color: gray; background: transparent; border: none; }");
        ui->btn_select_guest->setStyleSheet("QPushButton { color: #ffde59; font-weight: bold; background: transparent; border: none; }");

        ui->btn_join_room_ip_dots_and_boxes->setStyleSheet(
            "QPushButton { background-color: #ffde59; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }"
        );
    }
}

void DotsAndBoxesWindow::on_btn_select_host_clicked()
{
    setHostMode(true);
}

void DotsAndBoxesWindow::on_btn_select_guest_clicked()
{
    setHostMode(false);
}

void DotsAndBoxesWindow::on_btn_start_new_game_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_setup_dots_and_boxes);
    displayLocalIP();
}

void DotsAndBoxesWindow::on_btn_back_clicked()
{
    QWidget *parentMenu = this->parentWidget();
    if (parentMenu) {
        parentMenu->show();
    }
    this->close();
}

void DotsAndBoxesWindow::on_chk_time_limit_dots_and_boxes_stateChanged(int arg1)
{
    bool hasTimeLimit = (arg1 == Qt::Checked);
    ui->spin_time_min_dots_and_boxes->setEnabled(hasTimeLimit);
    ui->spin_time_sec_dots_and_boxes->setEnabled(hasTimeLimit);
}

void DotsAndBoxesWindow::on_btn_create_room_dots_and_boxes_clicked()
{
    QString boardSizeStr = ui->combo_board_size_dots_and_boxes->currentText();
    QString boardSize = boardSizeStr.split("x").first();

    int totalSeconds = 0;
    if (ui->chk_time_limit_dots_and_boxes->isChecked()) {
        totalSeconds = (ui->spin_time_min_dots_and_boxes->value() * 60) + ui->spin_time_sec_dots_and_boxes->value();
    }

    QString roomId = QString::fromStdString(currentUser.getUsername());
    QString payload = roomId + "|" + boardSize + "|" + QString::number(totalSeconds);

    ui->btn_create_room_dots_and_boxes->setText("Waiting...");
    ui->btn_create_room_dots_and_boxes->setEnabled(false);

    NetworkManager::instance().sendPacket(PacketType::CREATE_ROOM, QString::fromStdString(currentUser.getUsername()), payload);
}

void DotsAndBoxesWindow::on_btn_join_room_ip_dots_and_boxes_clicked()
{
    QString hostUsername = ui->txt_guest_ip_dots_and_boxes->text();

    if (hostUsername.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter the Host Username in the IP field.");
        return;
    }

    ui->btn_join_room_ip_dots_and_boxes->setText("Joining...");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(false);

    NetworkManager::instance().sendPacket(PacketType::JOIN_ROOM, QString::fromStdString(currentUser.getUsername()), hostUsername);
}

void DotsAndBoxesWindow::onLineClicked(int row, int col, bool isHorizontal)
{

}

void DotsAndBoxesWindow::onRoomJoined(QString message)
{
    if (message.contains("Waiting")) {

    } else {
        ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
    }
}

void DotsAndBoxesWindow::onGameStarted(QString message)
{
    ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
}

void DotsAndBoxesWindow::onErrorReceived(QString errorMsg)
{
    QMessageBox::warning(this, "Network Error", errorMsg);

    ui->btn_create_room_dots_and_boxes->setText("Create Room");
    ui->btn_create_room_dots_and_boxes->setEnabled(true);

    ui->btn_join_room_ip_dots_and_boxes->setText("Join Room");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(true);
}

void DotsAndBoxesWindow::displayLocalIP()
{
    QString localIP = "Unknown";
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            localIP = address.toString();
            break;
        }
    }
    ui->lbl_host_ip_dots_and_boxes->setText("YOUR LOCAL IP: " + localIP);
}
