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

    ui->board_widget->raise();
    ui->horizontalLayoutWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    setupDashboardUI();
    setupNetworkUI();

    m_turnTimer = new QTimer(this);
    connect(m_turnTimer, &QTimer::timeout, this, &DotsAndBoxesWindow::onTurnTimerTick);

    connect(ui->board_widget, &GameBoardWidget::lineClicked, this, &DotsAndBoxesWindow::onLineClicked);

    connect(&NetworkManager::instance(), &NetworkManager::roomJoined, this, &DotsAndBoxesWindow::onRoomJoined);
    connect(&NetworkManager::instance(), &NetworkManager::gameStarted, this, &DotsAndBoxesWindow::onGameStarted);
    connect(&NetworkManager::instance(), &NetworkManager::errorReceived, this, &DotsAndBoxesWindow::onErrorReceived);
    connect(&NetworkManager::instance(), SIGNAL(moveReceived(QString)), this, SLOT(onMoveReceived(QString)));
    connect(&NetworkManager::instance(), SIGNAL(turnChanged()), this, SLOT(onTurnChangedReceived()));
    connect(&NetworkManager::instance(), SIGNAL(gameOverReceived(QString)), this, SLOT(onGameOverReceived(QString)));
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
        "   border: 2px solid #00f0b5;"
        "   border-radius: 15px;"
        "   color: #00f0b5;"
        "   padding: 5px 15px;"
        "}"
    );

    ui->lbl_host_ip_dots_and_boxes->setStyleSheet(
        "QLabel {"
        "   border: 2px solid #00f0b5;"
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
        "   border: 3px solid #00f0b5;"
        "   border-radius: 25px;"
        "}";

    QString inactiveStyle =
        "QFrame#frame_host_setup, QFrame#frame_guest_setup {"
        "   background-color: rgba(20, 30, 48, 0.2);"
        "   border: 3px solid rgba(255, 255, 255, 0.1);"
        "   border-radius: 25px;"
        "}";

    QString activeGuestStyle =
        "QFrame#frame_guest_setup {"
        "   background-color: rgba(20, 30, 48, 0.6);"
        "   border: 3px solid #ffde59;"
        "   border-radius: 25px;"
        "}";

    ui->btn_create_room_dots_and_boxes->setText("Create Room");
    ui->btn_create_room_dots_and_boxes->setEnabled(true);
    ui->btn_join_room_ip_dots_and_boxes->setText("Join Room");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(true);

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
    setHostMode(true);
    ui->txt_host_port_dots_and_boxes->clear();
    ui->txt_guest_ip_dots_and_boxes->clear();
    ui->txt_guest_port_ip_dots_and_boxes->clear();
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
    if (ui->txt_host_port_dots_and_boxes->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter the Host Port.");
        return;
    }

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
    QString hostPort = ui->txt_guest_port_ip_dots_and_boxes->text();

    if (hostUsername.isEmpty() || hostPort.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please fill in both Host Username (Room ID) and Port fields.");
        return;
    }

    ui->btn_join_room_ip_dots_and_boxes->setText("Joining...");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(false);

    NetworkManager::instance().sendPacket(PacketType::JOIN_ROOM, QString::fromStdString(currentUser.getUsername()), hostUsername);
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

void DotsAndBoxesWindow::initGame(int boardSize, int timeLimit, bool isHost, QString opponent)
{
    m_boardSize = boardSize;
    m_timeLimit = timeLimit;
    m_opponentUsername = opponent;
    m_myPlayerId = isHost ? 1 : 2;
    m_isMyTurn = isHost;

    m_p1Score = 0;
    m_p2Score = 0;

    m_hLines.assign(m_boardSize, std::vector<int>(m_boardSize - 1, 0));
    m_vLines.assign(m_boardSize - 1, std::vector<int>(m_boardSize, 0));
    m_boxes.assign(m_boardSize - 1, std::vector<int>(m_boardSize - 1, 0));

    ui->board_widget->setBoardSize(m_boardSize);

    if (m_timeLimit > 0) {
        m_remainingTime = m_timeLimit;
        m_turnTimer->start(1000);
    }

    updateGameUI();
}

void DotsAndBoxesWindow::onRoomJoined(QString message)
{
    if (!message.contains("Waiting")) {
        QStringList parts = message.split("|");
        if (parts.size() >= 3) {
            initGame(parts[1].toInt(), parts[2].toInt(), false, parts[0]);
            ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
        }
    }
}

void DotsAndBoxesWindow::onGameStarted(QString message)
{
    QStringList parts = message.split("|");
    if (parts.size() >= 3) {
        initGame(parts[1].toInt(), parts[2].toInt(), true, parts[0]);
        ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
    }
}

void DotsAndBoxesWindow::onLineClicked(int r, int c, bool isHorizontal)
{
    if (!m_isMyTurn) return;

    if (isHorizontal) {
        if (m_hLines[r][c] != 0) return;
        m_hLines[r][c] = m_myPlayerId;
    } else {
        if (m_vLines[r][c] != 0) return;
        m_vLines[r][c] = m_myPlayerId;
    }

    QString moveStr = QString("%1 %2 %3").arg(isHorizontal ? "H" : "V").arg(r).arg(c);
    NetworkManager::instance().sendPacket(PacketType::MOVE_DOTS_BOXES, QString::fromStdString(currentUser.getUsername()), moveStr);

    int boxesClaimed = checkAndClaimBoxes(r, c, isHorizontal, m_myPlayerId);
    if (boxesClaimed > 0) {
        if (m_myPlayerId == 1) m_p1Score += boxesClaimed;
        else m_p2Score += boxesClaimed;
    } else {
        endTurn();
    }

    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    updateGameUI();
    checkGameOver();
}

void DotsAndBoxesWindow::onMoveReceived(QString moveData)
{
    QStringList parts = moveData.split(" ");
    if (parts.size() >= 3) {
        bool isHoriz = (parts[0] == "H");
        int r = parts[1].toInt();
        int c = parts[2].toInt();
        int oppId = (m_myPlayerId == 1) ? 2 : 1;

        if (isHoriz) m_hLines[r][c] = oppId;
        else m_vLines[r][c] = oppId;

        int boxesClaimed = checkAndClaimBoxes(r, c, isHoriz, oppId);
        if (boxesClaimed > 0) {
            if (oppId == 1) m_p1Score += boxesClaimed;
            else m_p2Score += boxesClaimed;
        }

        if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
        updateGameUI();
        checkGameOver();
    }
}

void DotsAndBoxesWindow::endTurn()
{
    m_isMyTurn = false;
    NetworkManager::instance().sendPacket(PacketType::TURN_CHANGE, QString::fromStdString(currentUser.getUsername()), "");
}

void DotsAndBoxesWindow::onTurnChangedReceived()
{
    m_isMyTurn = true;
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    updateGameUI();
}

int DotsAndBoxesWindow::checkAndClaimBoxes(int r, int c, bool isHorizontal, int playerId)
{
    int claimed = 0;
    if (isHorizontal) {
        if (r > 0 && m_boxes[r - 1][c] == 0) {
            if (m_hLines[r - 1][c] && m_vLines[r - 1][c] && m_vLines[r - 1][c + 1]) {
                m_boxes[r - 1][c] = playerId;
                claimed++;
            }
        }
        if (r < m_boardSize - 1 && m_boxes[r][c] == 0) {
            if (m_hLines[r + 1][c] && m_vLines[r][c] && m_vLines[r][c + 1]) {
                m_boxes[r][c] = playerId;
                claimed++;
            }
        }
    } else {
        if (c > 0 && m_boxes[r][c - 1] == 0) {
            if (m_vLines[r][c - 1] && m_hLines[r][c - 1] && m_hLines[r + 1][c - 1]) {
                m_boxes[r][c - 1] = playerId;
                claimed++;
            }
        }
        if (c < m_boardSize - 1 && m_boxes[r][c] == 0) {
            if (m_vLines[r][c + 1] && m_hLines[r][c] && m_hLines[r + 1][c]) {
                m_boxes[r][c] = playerId;
                claimed++;
            }
        }
    }
    return claimed;
}

void DotsAndBoxesWindow::updateGameUI()
{
    ui->lbl_player1_score->setText(QString("P1 (Host): %1").arg(m_p1Score));
    ui->lbl_player2_score->setText(QString("P2 (Guest): %1").arg(m_p2Score));

    QString turnText = m_isMyTurn ? "YOUR TURN" : "OPPONENT'S TURN";
    if (m_timeLimit > 0) {
        turnText += QString("\nTime: %1s").arg(m_remainingTime);
    }
    ui->lbl_turn->setText(turnText);

    if (m_isMyTurn) {
        ui->lbl_turn->setStyleSheet("color: #00ffcc; font-weight: bold; font-size: 18px; text-align: center;");
    } else {
        ui->lbl_turn->setStyleSheet("color: #ff4d6d; font-weight: bold; font-size: 18px; text-align: center;");
    }

    ui->board_widget->updateBoardState(m_hLines, m_vLines, m_boxes);
}

void DotsAndBoxesWindow::onTurnTimerTick()
{
    if (m_remainingTime > 0) {
        m_remainingTime--;
        updateGameUI();
    } else {
        if (m_isMyTurn) {
            endTurn();
            updateGameUI();
        }
    }
}

void DotsAndBoxesWindow::checkGameOver()
{
    int totalBoxes = (m_boardSize - 1) * (m_boardSize - 1);
    if (m_p1Score + m_p2Score == totalBoxes) {
        if (m_turnTimer->isActive()) m_turnTimer->stop();
        QString winner = (m_p1Score > m_p2Score) ? "P1 Wins!" : ((m_p2Score > m_p1Score) ? "P2 Wins!" : "Draw!");
        QMessageBox::information(this, "Game Over", "Game finished!\nResult: " + winner);
        ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    }
}

void DotsAndBoxesWindow::onGameOverReceived(QString message)
{
    if (m_turnTimer->isActive()) {
        m_turnTimer->stop();
    }
    QMessageBox::information(this, "Game Over", "The match has ended (Opponent left).");
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void DotsAndBoxesWindow::on_btn_back_to_dashboard_clicked()
{
    if (m_turnTimer->isActive()) {
        m_turnTimer->stop();
    }
    QString payload = "0|" + m_opponentUsername + "|" + QString::fromStdString(currentUser.getUsername()) + "|" + QString::number(m_p2Score);
    NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void DotsAndBoxesWindow::onErrorReceived(QString errorMsg)
{
    QMessageBox::warning(this, "Network Error", errorMsg);
    ui->btn_create_room_dots_and_boxes->setText("Create Room");
    ui->btn_create_room_dots_and_boxes->setEnabled(true);
    ui->btn_join_room_ip_dots_and_boxes->setText("Join Room");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(true);
}
