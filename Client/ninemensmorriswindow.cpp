#include "ninemensmorriswindow.h"
#include "ui_ninemensmorriswindow.h"
<<<<<<< HEAD

NineMensMorrisWindow::NineMensMorrisWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NineMensMorrisWindow)
{
    ui->setupUi(this);
=======
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDateTime>
#include "networkmanager.h"
#include "mainmenu.h"

NineMensMorrisWindow::NineMensMorrisWindow(const User& user, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NineMensMorrisWindow),
    currentUser(user)
{
    ui->setupUi(this);

    m_localServerProcess = nullptr;
    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;
    m_isGameOver = false;

    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    ui->spin_time_min->setEnabled(false);
    ui->spin_time_sec->setEnabled(false);
    ui->lbl_score->setText(QString::number(currentUser.getNineMensMorrisScore()));

    setupDashboardUI();
    setupNetworkUI();

    m_turnTimer = new QTimer(this);
    connect(m_turnTimer, &QTimer::timeout, this, &NineMensMorrisWindow::onTurnTimerTick);

    connect(ui->board_widget, &NineMensBoardWidget::positionClicked, this, &NineMensMorrisWindow::onPositionClicked);

    connect(&NetworkManager::instance(), &NetworkManager::roomJoined, this, &NineMensMorrisWindow::onRoomJoined);
    connect(&NetworkManager::instance(), &NetworkManager::gameStarted, this, &NineMensMorrisWindow::onGameStarted);
    connect(&NetworkManager::instance(), &NetworkManager::errorReceived, this, &NineMensMorrisWindow::onErrorReceived);
    connect(&NetworkManager::instance(), &NetworkManager::connectionError, this, &NineMensMorrisWindow::onConnectionError);
    connect(&NetworkManager::instance(), SIGNAL(moveReceived(QString)), this, SLOT(onMoveReceived(QString)));
    connect(&NetworkManager::instance(), SIGNAL(turnChanged()), this, SLOT(onTurnChangedReceived()));
    connect(&NetworkManager::instance(), SIGNAL(gameOverReceived(QString)), this, SLOT(onGameOverReceived(QString)));

    connect(&NetworkManager::instance(), &NetworkManager::connectedToServer, this, &NineMensMorrisWindow::onServerConnected);
>>>>>>> efd0b59a9cf09aab42d0206f7e8bbf4830b244f5
}

NineMensMorrisWindow::~NineMensMorrisWindow()
{
<<<<<<< HEAD
    delete ui;
}
=======
    cleanupNetworkAndServer();
    delete ui;
}

void NineMensMorrisWindow::cleanupNetworkAndServer()
{
    NetworkManager::instance().disconnectFromServer();

    if (m_localServerProcess) {
        QProcess *proc = m_localServerProcess;
        m_localServerProcess = nullptr;

        QTimer::singleShot(1000, proc, [proc]() {
            proc->kill();
            proc->waitForFinished();
            proc->deleteLater();
        });
    }

    QTimer::singleShot(1100, this, [=]() {
        NetworkManager::instance().connectToServer("127.0.0.1", 12345);
    });
}

void NineMensMorrisWindow::setupDashboardUI()
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
        if (record.gameName == GameType::NineMensMorris) {
            int row = ui->tbl_history->rowCount();
            ui->tbl_history->insertRow(row);

            ui->tbl_history->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.opponent)));
            ui->tbl_history->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.date)));
            ui->tbl_history->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.playerRole)));

            QTableWidgetItem *resultItem = new QTableWidgetItem(QString::fromStdString(record.result));
            if (record.result.find("Win") != std::string::npos || record.result == "WIN") {
                resultItem->setForeground(QColor("#00ffcc"));
            } else if (record.result.find("Draw") != std::string::npos) {
                resultItem->setForeground(QColor("#ffde59"));
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

void NineMensMorrisWindow::setupNetworkUI()
{
    ui->txt_host_port->setPlaceholderText("ENTER HOST PORT");
    ui->txt_guest_ip->setPlaceholderText("ENTER HOST IP");
    ui->txt_guest_port->setPlaceholderText("ENTER HOST PORT");

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

    ui->txt_host_port->setStyleSheet(lineEditStyle);
    ui->txt_guest_ip->setStyleSheet(lineEditStyle);
    ui->txt_guest_port->setStyleSheet(lineEditStyle);

    ui->lbl_host_ip->setStyleSheet(
        "QLabel {"
        "   border: 2px solid #00f0b5;"
        "   border-radius: 15px;"
        "   color: #00f0b5;"
        "   padding: 5px;"
        "}"
    );

    connect(ui->btn_select_host, &QPushButton::clicked, this, &NineMensMorrisWindow::on_btn_select_host_clicked);
    connect(ui->btn_select_guest, &QPushButton::clicked, this, &NineMensMorrisWindow::on_btn_select_guest_clicked);

    setHostMode(true);
}

void NineMensMorrisWindow::setHostMode(bool isHost)
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

    ui->btn_create_room->setText("Create Room");
    ui->btn_create_room->setEnabled(true);
    ui->btn_join_room->setText("Join Room");
    ui->btn_join_room->setEnabled(true);

    if (isHost) {
        ui->frame_host_setup->setStyleSheet(activeHostStyle);
        ui->frame_guest_setup->setStyleSheet(inactiveStyle);
        ui->frame_host_setup->setEnabled(true);
        ui->frame_guest_setup->setEnabled(false);

        ui->btn_select_host->setStyleSheet("QPushButton { color: #00f0b5; font-weight: bold; background: transparent; border: none; }");
        ui->btn_select_guest->setStyleSheet("QPushButton { color: gray; background: transparent; border: none; }");

        ui->btn_create_room->setStyleSheet(
            "QPushButton { background-color: #00f0b5; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }"
        );
    } else {
        ui->frame_host_setup->setStyleSheet(inactiveStyle);
        ui->frame_guest_setup->setStyleSheet(activeGuestStyle);
        ui->frame_host_setup->setEnabled(false);
        ui->frame_guest_setup->setEnabled(true);

        ui->btn_select_host->setStyleSheet("QPushButton { color: gray; background: transparent; border: none; }");
        ui->btn_select_guest->setStyleSheet("QPushButton { color: #ffde59; font-weight: bold; background: transparent; border: none; }");

        ui->btn_join_room->setStyleSheet(
            "QPushButton { background-color: #ffde59; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }"
        );
    }
}

void NineMensMorrisWindow::on_btn_select_host_clicked()
{
    setHostMode(true);
}

void NineMensMorrisWindow::on_btn_select_guest_clicked()
{
    setHostMode(false);
}

void NineMensMorrisWindow::on_btn_start_new_game_clicked()
{
    setHostMode(true);
    ui->txt_host_port->clear();
    ui->txt_guest_ip->clear();
    ui->txt_guest_port->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_setup);
    displayLocalIP();
}

void NineMensMorrisWindow::on_btn_back_clicked()
{
    cleanupNetworkAndServer();

    MainMenu *parentMenu = qobject_cast<MainMenu*>(this->parentWidget());
    if (parentMenu) {
        parentMenu->updateUserData(currentUser);
        parentMenu->show();
    }

    this->close();
}

void NineMensMorrisWindow::on_chk_time_limit_stateChanged(int arg1)
{
    bool hasTimeLimit = (arg1 == Qt::Checked);
    ui->spin_time_min->setEnabled(hasTimeLimit);
    ui->spin_time_sec->setEnabled(hasTimeLimit);
}

void NineMensMorrisWindow::on_btn_create_room_clicked()
{
    if (ui->txt_host_port->text().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter the Host Port.");
        return;
    }

    QString portStr = ui->txt_host_port->text();

    ui->btn_create_room->setText("Starting Server...");
    ui->btn_create_room->setEnabled(false);

    if (m_localServerProcess) {
        m_localServerProcess->kill();
        m_localServerProcess->waitForFinished();
        delete m_localServerProcess;
    }
    m_localServerProcess = new QProcess(this);
    m_localServerProcess->start("PlaTwo_Server.exe", QStringList() << portStr);

    NetworkManager::instance().disconnectFromServer();
    m_pendingCreateRoom = true;

    QTimer::singleShot(500, this, [=]() {
        NetworkManager::instance().connectToServer("127.0.0.1", portStr.toUShort());
    });
}

void NineMensMorrisWindow::on_btn_join_room_clicked()
{
    QString hostIp = ui->txt_guest_ip->text();
    QString hostPort = ui->txt_guest_port->text();

    if (hostIp.isEmpty() || hostPort.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please fill in both Host IP and Port fields.");
        return;
    }

    ui->btn_join_room->setText("Joining...");
    ui->btn_join_room->setEnabled(false);

    NetworkManager::instance().disconnectFromServer();
    m_pendingJoinRoom = true;
    NetworkManager::instance().connectToServer(hostIp, hostPort.toUShort());
}

void NineMensMorrisWindow::onServerConnected()
{
    if (m_pendingCreateRoom) {
        m_pendingCreateRoom = false;

        int totalSeconds = 0;
        if (ui->chk_time_limit->isChecked()) {
            totalSeconds = (ui->spin_time_min->value() * 60) + ui->spin_time_sec->value();
        }

        QString roomId = QString::fromStdString(currentUser.getUsername());
        QString payload = roomId + "|6|" + QString::number(totalSeconds);

        ui->btn_create_room->setText("Waiting for Guest...");
        NetworkManager::instance().sendPacket(PacketType::CREATE_ROOM, QString::fromStdString(currentUser.getUsername()), payload);
    }
    else if (m_pendingJoinRoom) {
        m_pendingJoinRoom = false;
        NetworkManager::instance().sendPacket(PacketType::JOIN_ROOM, QString::fromStdString(currentUser.getUsername()), "JOIN_ANY_ROOM");
    }
}

void NineMensMorrisWindow::displayLocalIP()
{
    QString localIP = "Unknown";
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            localIP = address.toString();
            break;
        }
    }
    ui->lbl_host_ip->setText("YOUR LOCAL IP: " + localIP);
}

void NineMensMorrisWindow::initGame(bool isHost, QString opponent, int timeLimit)
{
    m_timeLimit = timeLimit;
    m_opponentUsername = opponent;
    m_myPlayerId = isHost ? 1 : 2;
    m_isMyTurn = isHost;
    m_isGameOver = false;

    m_board.assign(24, 0);
    m_p1Unplaced = 9;
    m_p2Unplaced = 9;
    m_p1Count = 0;
    m_p2Count = 0;
    m_isRemovingState = false;
    m_selectedPos = -1;

    ui->board_widget->setBoardState(m_board);

    if (m_timeLimit > 0) {
        m_remainingTime = m_timeLimit;
        m_turnTimer->start(1000);
    }

    updateGameUI();
}

void NineMensMorrisWindow::onRoomJoined(QString message)
{
    if (!message.contains("Waiting")) {
        QStringList parts = message.split("|");
        if (parts.size() >= 3) {
            initGame(false, parts[0], parts[2].toInt());
            ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
        }
    }
}

void NineMensMorrisWindow::onGameStarted(QString message)
{
    QStringList parts = message.split("|");
    if (parts.size() >= 3) {
        initGame(true, parts[0], parts[2].toInt());
        ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
    }
}

bool NineMensMorrisWindow::isAdjacent(int from, int to)
{
    static const std::vector<std::vector<int>> adj = {
        {1, 7}, {0, 2, 9}, {1, 3}, {2, 4, 11}, {3, 5}, {4, 6, 13}, {5, 7}, {6, 0, 15},
        {9, 15}, {8, 10, 1, 17}, {9, 11}, {10, 12, 3, 19}, {11, 13}, {12, 14, 5, 21}, {13, 15}, {14, 8, 7, 23},
        {17, 23}, {16, 18, 9}, {17, 19}, {18, 20, 11}, {19, 21}, {20, 22, 13}, {21, 23}, {22, 16, 15}
    };

    for (int neighbor : adj[from]) {
        if (neighbor == to) return true;
    }
    return false;
}

bool NineMensMorrisWindow::hasLegalMoves(int player)
{
    int unplaced = (player == 1) ? m_p1Unplaced : m_p2Unplaced;
    int count = (player == 1) ? m_p1Count : m_p2Count;

    if (unplaced > 0) return true;
    if (count <= 3) return true;

    static const std::vector<std::vector<int>> adj = {
        {1, 7}, {0, 2, 9}, {1, 3}, {2, 4, 11}, {3, 5}, {4, 6, 13}, {5, 7}, {6, 0, 15},
        {9, 15}, {8, 10, 1, 17}, {9, 11}, {10, 12, 3, 19}, {11, 13}, {12, 14, 5, 21}, {13, 15}, {14, 8, 7, 23},
        {17, 23}, {16, 18, 9}, {17, 19}, {18, 20, 11}, {19, 21}, {20, 22, 13}, {21, 23}, {22, 16, 15}
    };

    for (int i = 0; i < 24; ++i) {
        if (m_board[i] == player) {
            for (int neighbor : adj[i]) {
                if (m_board[neighbor] == 0) return true;
            }
        }
    }
    return false;
}

bool NineMensMorrisWindow::checkMill(int pos, int player)
{
    static const std::vector<std::vector<int>> mills = {
        {0,1,2}, {2,3,4}, {4,5,6}, {6,7,0},
        {8,9,10}, {10,11,12}, {12,13,14}, {14,15,8},
        {16,17,18}, {18,19,20}, {20,21,22}, {22,23,16},
        {1,9,17}, {3,11,19}, {5,13,21}, {7,15,23}
    };

    for (const auto& m : mills) {
        if (m[0] == pos || m[1] == pos || m[2] == pos) {
            if (m_board[m[0]] == player && m_board[m[1]] == player && m_board[m[2]] == player) {
                return true;
            }
        }
    }
    return false;
}

void NineMensMorrisWindow::onPositionClicked(int posIndex)
{
    if (!m_isMyTurn || m_isGameOver) return;

    int myUnplaced = (m_myPlayerId == 1) ? m_p1Unplaced : m_p2Unplaced;
    int oppPlayerId = (m_myPlayerId == 1) ? 2 : 1;

    if (m_isRemovingState) {
        if (m_board[posIndex] == oppPlayerId) {

            bool isPieceInMill = checkMill(posIndex, oppPlayerId);
            bool allInMill = true;
            for(int i = 0; i < 24; i++) {
                if (m_board[i] == oppPlayerId && !checkMill(i, oppPlayerId)) {
                    allInMill = false;
                    break;
                }
            }
            if (isPieceInMill && !allInMill) {
                QMessageBox::warning(this, "Invalid Move", "شما نمی‌توانید مهره‌ای را که داخل دوز است بزنید، مگر اینکه تمام مهره‌های حریف داخل دوز باشند.");
                return;
            }

            m_board[posIndex] = 0;
            if (oppPlayerId == 1) m_p1Count--;
            else m_p2Count--;

            m_isRemovingState = false;
            ui->board_widget->setSelectedPosition(-1);

            QString moveStr = QString("REMOVE %1").arg(posIndex);
            NetworkManager::instance().sendPacket(PacketType::MOVE_NINE_MENS, QString::fromStdString(currentUser.getUsername()), moveStr);

            if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
            endTurn();
            updateGameUI();
            checkGameOver();
        }
        return;
    }

    if (myUnplaced > 0) {
        if (m_board[posIndex] == 0) {
            m_board[posIndex] = m_myPlayerId;
            if (m_myPlayerId == 1) { m_p1Unplaced--; m_p1Count++; }
            else { m_p2Unplaced--; m_p2Count++; }

            QString moveStr = QString("PLACE %1").arg(posIndex);
            NetworkManager::instance().sendPacket(PacketType::MOVE_NINE_MENS, QString::fromStdString(currentUser.getUsername()), moveStr);

            if (m_timeLimit > 0) m_remainingTime = m_timeLimit;

            if (checkMill(posIndex, m_myPlayerId)) {
                m_isRemovingState = true;
                updateGameUI();
            } else {
                endTurn();
                updateGameUI();
            }
        }
    }
    else {
        int myPieceCount = (m_myPlayerId == 1) ? m_p1Count : m_p2Count;

        if (m_selectedPos == -1) {
            if (m_board[posIndex] == m_myPlayerId) {
                m_selectedPos = posIndex;
                ui->board_widget->setSelectedPosition(m_selectedPos);
            }
        } else {
            if (posIndex == m_selectedPos) {
                m_selectedPos = -1;
                ui->board_widget->setSelectedPosition(-1);
            } else if (m_board[posIndex] == 0 && (isAdjacent(m_selectedPos, posIndex) || myPieceCount == 3)) {
                m_board[m_selectedPos] = 0;
                m_board[posIndex] = m_myPlayerId;

                QString moveStr = QString("MOVE %1 %2").arg(m_selectedPos).arg(posIndex);
                NetworkManager::instance().sendPacket(PacketType::MOVE_NINE_MENS, QString::fromStdString(currentUser.getUsername()), moveStr);

                int movedPos = posIndex;
                m_selectedPos = -1;
                ui->board_widget->setSelectedPosition(-1);

                if (m_timeLimit > 0) m_remainingTime = m_timeLimit;

                if (checkMill(movedPos, m_myPlayerId)) {
                    m_isRemovingState = true;
                    updateGameUI();
                } else {
                    endTurn();
                    updateGameUI();
                }
            }
        }
    }
}

void NineMensMorrisWindow::onMoveReceived(QString moveData)
{
    QStringList parts = moveData.split(" ");
    int oppId = (m_myPlayerId == 1) ? 2 : 1;

    if (parts[0] == "PLACE" && parts.size() >= 2) {
        int pos = parts[1].toInt();
        m_board[pos] = oppId;
        if (oppId == 1) { m_p1Unplaced--; m_p1Count++; }
        else { m_p2Unplaced--; m_p2Count++; }
    }
    else if (parts[0] == "MOVE" && parts.size() >= 3) {
        int from = parts[1].toInt();
        int to = parts[2].toInt();
        m_board[from] = 0;
        m_board[to] = oppId;
    }
    else if (parts[0] == "REMOVE" && parts.size() >= 2) {
        int pos = parts[1].toInt();
        m_board[pos] = 0;
        if (m_myPlayerId == 1) m_p1Count--;
        else m_p2Count--;
    }

    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;

    updateGameUI();
    checkGameOver();
}

void NineMensMorrisWindow::endTurn()
{
    m_isMyTurn = false;
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    NetworkManager::instance().sendPacket(PacketType::TURN_CHANGE, QString::fromStdString(currentUser.getUsername()), "");
}

void NineMensMorrisWindow::onTurnChangedReceived()
{
    m_isMyTurn = true;
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    updateGameUI();
}

void NineMensMorrisWindow::updateGameUI()
{
    int myUnplaced = (m_myPlayerId == 1) ? m_p1Unplaced : m_p2Unplaced;
    int oppUnplaced = (m_myPlayerId == 1) ? m_p2Unplaced : m_p1Unplaced;

    ui->lbl_p1_info->setText(QString("P1 Unplaced: %1 | On Board: %2").arg(m_p1Unplaced).arg(m_p1Count));
    ui->lbl_p2_info->setText(QString("P2 Unplaced: %1 | On Board: %2").arg(m_p2Unplaced).arg(m_p2Count));

    QString turnText = m_isMyTurn ? "YOUR TURN" : "OPPONENT'S TURN";
    if (m_isRemovingState) turnText = "REMOVE OPPONENT PIECE!";
    if (m_timeLimit > 0) turnText += QString("\nTime: %1s").arg(m_remainingTime);

    ui->lbl_turn->setText(turnText);

    if (m_isMyTurn) {
        ui->lbl_turn->setStyleSheet("color: #00ffcc; font-weight: bold; font-size: 18px; text-align: center;");
    } else {
        ui->lbl_turn->setStyleSheet("color: #ff4d6d; font-weight: bold; font-size: 18px; text-align: center;");
    }

    ui->board_widget->setBoardState(m_board);
}

void NineMensMorrisWindow::onTurnTimerTick()
{
    if (m_remainingTime > 0) {
        m_remainingTime--;
        updateGameUI();
    } else {
        if (m_isMyTurn) {
            m_turnTimer->stop();
            QMessageBox::warning(this, "Time's up!", "You lose");
            on_btn_back_to_dashboard_clicked();
        }
    }
}

void NineMensMorrisWindow::checkGameOver()
{
    if (m_isGameOver) return;

    if (m_p1Unplaced == 0 && m_p2Unplaced == 0) {
        bool gameOver = false;
        QString winnerText = "";
        QString myResult = "Draw";

        bool p1Loses = (m_p1Count < 3) || !hasLegalMoves(1);
        bool p2Loses = (m_p2Count < 3) || !hasLegalMoves(2);

        if (p1Loses) {
            gameOver = true;
            winnerText = "P2 Wins! (P1 is blocked or has less than 3 pieces)";
            myResult = (m_myPlayerId == 2) ? "Win" : "Loss";
        } else if (p2Loses) {
            gameOver = true;
            winnerText = "P1 Wins! (P2 is blocked or has less than 3 pieces)";
            myResult = (m_myPlayerId == 1) ? "Win" : "Loss";
        }

        if (gameOver) {
            if (m_turnTimer->isActive()) m_turnTimer->stop();
            m_isGameOver = true;

            int score = (myResult == "Win") ? 100 : 0;

            GameRecord newRecord;
            newRecord.gameName = GameType::NineMensMorris;
            newRecord.opponent = m_opponentUsername.toStdString();
            newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
            newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
            newRecord.result = myResult.toStdString();
            newRecord.score = score;

            if (score > 0) currentUser.updateScore(GameType::NineMensMorris, score);
            currentUser.addGameRecord(newRecord);

            ui->lbl_score->setText(QString::number(currentUser.getNineMensMorrisScore()));
            setupDashboardUI();

            if (m_myPlayerId == 1) {
                QString payload = QString::number(static_cast<int>(GameType::NineMensMorris)) + "|" +
                                  QString::fromStdString(currentUser.getUsername()) + "|" +
                                  QString::number(m_myPlayerId == 1 && myResult == "Win" ? 100 : 0) + "|" +
                                  (myResult == "Win" ? "Win" : "Loss") + "|" +
                                  m_opponentUsername + "|" +
                                  QString::number(m_myPlayerId == 2 && myResult == "Win" ? 100 : 0) + "|" +
                                  (myResult == "Win" ? "Loss" : "Win");

                NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);
            }

            QMessageBox::information(this, "Game Over", "Game finished!\n" + winnerText);

            cleanupNetworkAndServer();
            ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
        }
    }
}

void NineMensMorrisWindow::onGameOverReceived(QString message)
{
    if (m_isGameOver) return;

    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    GameRecord newRecord;
    newRecord.gameName = GameType::NineMensMorris;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = "Win (Opponent Surrendered/Timeout)";
    newRecord.score = 100;

    currentUser.updateScore(GameType::NineMensMorris, 100);
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getNineMensMorrisScore()));
    setupDashboardUI();

    QMessageBox::information(this, "Game Over", "The opponent surrendered or ran out of time!\nYou win and earned 100 points!");

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void NineMensMorrisWindow::on_btn_back_to_dashboard_clicked()
{
    if (m_isGameOver) return;

    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    QString payload = QString::number(static_cast<int>(GameType::NineMensMorris)) + "|" +
                      QString::fromStdString(currentUser.getUsername()) + "|0|Loss (Surrendered)|" +
                      m_opponentUsername + "|100|Win (Opponent Surrendered)";

    NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);

    GameRecord newRecord;
    newRecord.gameName = GameType::NineMensMorris;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = "Loss (Surrendered/Timeout)";
    newRecord.score = 0;
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getNineMensMorrisScore()));
    setupDashboardUI();

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void NineMensMorrisWindow::onConnectionError(QString errorMsg)
{
    if (m_isGameOver) return;
    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;

    QMessageBox::warning(this, "Connection Error", "Cannot connect to the Room! Please check the IP and Port.");

    ui->btn_create_room->setText("Create Room");
    ui->btn_create_room->setEnabled(true);
    ui->btn_join_room->setText("Join Room");
    ui->btn_join_room->setEnabled(true);
}

void NineMensMorrisWindow::onErrorReceived(QString errorMsg)
{
    if (m_isGameOver) return;

    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;

    QMessageBox::warning(this, "Network Error", errorMsg);

    if (ui->stackedWidget->currentWidget() == ui->page_2_gameplay) {
        cleanupNetworkAndServer();
        ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    } else {
        ui->btn_create_room->setText("Create Room");
        ui->btn_create_room->setEnabled(true);
        ui->btn_join_room->setText("Join Room");
        ui->btn_join_room->setEnabled(true);
    }
}
>>>>>>> efd0b59a9cf09aab42d0206f7e8bbf4830b244f5
