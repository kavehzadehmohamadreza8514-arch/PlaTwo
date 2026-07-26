#include "dotsandboxeswindow.h"
#include "ui_dotsandboxeswindow.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDateTime>
#include "networkmanager.h"
#include "mainmenu.h"

DotsAndBoxesWindow::DotsAndBoxesWindow(const User& user, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DotsAndBoxesWindow),
    currentUser(user)
{
    ui->setupUi(this);

    m_localServerProcess = nullptr;
    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;
    m_isGameOver = false;

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
    connect(&NetworkManager::instance(), &NetworkManager::connectionError, this, &DotsAndBoxesWindow::onConnectionError);
    connect(&NetworkManager::instance(), SIGNAL(moveReceived(QString)), this, SLOT(onMoveReceived(QString)));
    connect(&NetworkManager::instance(), SIGNAL(turnChanged()), this, SLOT(onTurnChangedReceived()));
    connect(&NetworkManager::instance(), SIGNAL(gameOverReceived(QString)), this, SLOT(onGameOverReceived(QString)));

    connect(&NetworkManager::instance(), &NetworkManager::connectedToServer, this, &DotsAndBoxesWindow::onServerConnected);
}

DotsAndBoxesWindow::~DotsAndBoxesWindow()
{
    cleanupNetworkAndServer();
    delete ui;
}

void DotsAndBoxesWindow::cleanupNetworkAndServer()
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

void DotsAndBoxesWindow::setupNetworkUI()
{
    ui->txt_host_port_dots_and_boxes->setPlaceholderText("ENTER HOST PORT");
    ui->txt_guest_ip_dots_and_boxes->setPlaceholderText("ENTER HOST IP");
    ui->txt_guest_port_ip_dots_and_boxes->setPlaceholderText("ENTER HOST PORT");

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

    QLabel* lbl_host_color = new QLabel("Color:", ui->frame_host_setup);
    lbl_host_color->setGeometry(40, 115, 60, 31);
    lbl_host_color->setStyleSheet("color:rgb(255, 170, 255); font-size:12pt; font-weight:bold;");

    combo_host_color = new QComboBox(ui->frame_host_setup);
    combo_host_color->setGeometry(100, 115, 140, 31);
    combo_host_color->setStyleSheet(ui->combo_board_size_dots_and_boxes->styleSheet());

    QLabel* lbl_guest_color = new QLabel("Color:", ui->frame_guest_setup);
    lbl_guest_color->setGeometry(40, 210, 60, 31);
    lbl_guest_color->setStyleSheet("color:rgb(255, 170, 255); font-size:12pt; font-weight:bold;");

    combo_guest_color = new QComboBox(ui->frame_guest_setup);
    combo_guest_color->setGeometry(100, 210, 140, 31);
    combo_guest_color->setStyleSheet(ui->combo_board_size_dots_and_boxes->styleSheet());

    QStringList colors = {"Green", "Red", "Blue", "Yellow", "Cyan", "Magenta", "Orange", "Purple", "White"};
    combo_host_color->addItems(colors);
    combo_guest_color->addItems(colors);
    combo_host_color->setCurrentText("Green");
    combo_guest_color->setCurrentText("Red");

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
    cleanupNetworkAndServer();

    MainMenu *parentMenu = qobject_cast<MainMenu*>(this->parentWidget());
    if (parentMenu) {
        parentMenu->updateUserData(currentUser);
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

    QString portStr = ui->txt_host_port_dots_and_boxes->text();

    ui->btn_create_room_dots_and_boxes->setText("Starting Server...");
    ui->btn_create_room_dots_and_boxes->setEnabled(false);

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

void DotsAndBoxesWindow::on_btn_join_room_ip_dots_and_boxes_clicked()
{
    QString hostIp = ui->txt_guest_ip_dots_and_boxes->text();
    QString hostPort = ui->txt_guest_port_ip_dots_and_boxes->text();

    if (hostIp.isEmpty() || hostPort.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please fill in both Host IP and Port fields.");
        return;
    }

    ui->btn_join_room_ip_dots_and_boxes->setText("Joining...");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(false);

    NetworkManager::instance().disconnectFromServer();
    m_pendingJoinRoom = true;
    NetworkManager::instance().connectToServer(hostIp, hostPort.toUShort());
}

void DotsAndBoxesWindow::onServerConnected()
{
    if (m_pendingCreateRoom) {
        m_pendingCreateRoom = false;
        QString boardSizeStr = ui->combo_board_size_dots_and_boxes->currentText();
        QString boardSize = boardSizeStr.split("x").first();

        int totalSeconds = 0;
        if (ui->chk_time_limit_dots_and_boxes->isChecked()) {
            totalSeconds = (ui->spin_time_min_dots_and_boxes->value() * 60) + ui->spin_time_sec_dots_and_boxes->value();
        }

        QString roomId = QString::fromStdString(currentUser.getUsername());

        QString payload = roomId + "|" + boardSize + "|" + QString::number(totalSeconds) + "|" + combo_host_color->currentText();

        ui->btn_create_room_dots_and_boxes->setText("Waiting for Guest...");
        NetworkManager::instance().sendPacket(PacketType::CREATE_ROOM, QString::fromStdString(currentUser.getUsername()), payload);
    }
    else if (m_pendingJoinRoom) {
        m_pendingJoinRoom = false;

        QString payload = "JOIN_ANY_ROOM|" + combo_guest_color->currentText();
        NetworkManager::instance().sendPacket(PacketType::JOIN_ROOM, QString::fromStdString(currentUser.getUsername()), payload);
    }
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

QColor DotsAndBoxesWindow::getColorFromString(const QString& colorName) {
    if (colorName == "Green") return QColor("#00f0b5");
    if (colorName == "Red") return QColor("#ff4d6d");
    if (colorName == "Blue") return QColor("#0077ff");
    if (colorName == "Yellow") return QColor("#ffde59");
    if (colorName == "Cyan") return QColor("#00ffff");
    if (colorName == "Magenta") return QColor("#ff00ff");
    if (colorName == "Orange") return QColor("#ff9100");
    if (colorName == "Purple") return QColor("#8a2be2");
    if (colorName == "White") return QColor("#ffffff");
    return QColor("#00f0b5"); // Fallback
}

void DotsAndBoxesWindow::initGame(int boardSize, int timeLimit, bool isHost, QString opponent, QString hostColorStr, QString guestColorStr)
{
    m_boardSize = boardSize;
    m_timeLimit = timeLimit;
    m_opponentUsername = opponent;
    m_myPlayerId = isHost ? 1 : 2;
    m_isMyTurn = isHost;
    m_isGameOver = false;

    m_p1Score = 0;
    m_p2Score = 0;

    m_hLines.assign(m_boardSize, std::vector<int>(m_boardSize - 1, 0));
    m_vLines.assign(m_boardSize - 1, std::vector<int>(m_boardSize, 0));
    m_boxes.assign(m_boardSize - 1, std::vector<int>(m_boardSize - 1, 0));

    ui->board_widget->setBoardSize(m_boardSize);

    QString myUsername = QString::fromStdString(currentUser.getUsername());
    QString p1Name = isHost ? myUsername : opponent;
    QString p2Name = isHost ? opponent : myUsername;
    ui->board_widget->setPlayerNames(p1Name, p2Name);

    ui->board_widget->setPlayerColors(getColorFromString(hostColorStr), getColorFromString(guestColorStr));

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
        if (parts.size() >= 5) {
            initGame(parts[1].toInt(), parts[2].toInt(), false, parts[0], parts[3], parts[4]);
            ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay);
        }
    }
}

void DotsAndBoxesWindow::onGameStarted(QString message)
{
    QStringList parts = message.split("|");
    if (parts.size() >= 4) {
        initGame(parts[1].toInt(), parts[2].toInt(), true, parts[0], combo_host_color->currentText(), parts[3]);
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
            m_turnTimer->stop();
            QMessageBox::warning(this, "Time's up!", "Your time is up! You lose.");
            on_btn_back_to_dashboard_clicked();
        }
    }
}

void DotsAndBoxesWindow::checkGameOver()
{
    int totalBoxes = (m_boardSize - 1) * (m_boardSize - 1);

    if (m_p1Score + m_p2Score == totalBoxes) {
        if (m_turnTimer->isActive()) m_turnTimer->stop();
        m_isGameOver = true;

        QString winnerText;
        QString myResult = "Draw";
        QString oppResult = "Draw";

        if (m_p1Score > m_p2Score) {
            winnerText = "P1 (Host) Wins!";
            if (m_myPlayerId == 1) { myResult = "Win"; oppResult = "Loss"; }
            else { myResult = "Loss"; oppResult = "Win"; }
        } else if (m_p2Score > m_p1Score) {
            winnerText = "P2 (Guest) Wins!";
            if (m_myPlayerId == 2) { myResult = "Win"; oppResult = "Loss"; }
            else { myResult = "Loss"; oppResult = "Win"; }
        } else {
            winnerText = "Draw!";
        }

        int myEarnedScore = (m_myPlayerId == 1) ? m_p1Score : m_p2Score;
        int oppEarnedScore = (m_myPlayerId == 1) ? m_p2Score : m_p1Score;

        GameRecord newRecord;
        newRecord.gameName = GameType::DotsAndBoxes;
        newRecord.opponent = m_opponentUsername.toStdString();
        newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
        newRecord.result = myResult.toStdString();
        newRecord.score = myEarnedScore;

        if (myEarnedScore > 0) currentUser.updateScore(GameType::DotsAndBoxes, myEarnedScore);
        currentUser.addGameRecord(newRecord);

        ui->lbl_score->setText(QString::number(currentUser.getDotsAndBoxesScore()));
        setupDashboardUI();

        if (m_myPlayerId == 1) {
            QString payload = QString::number(static_cast<int>(GameType::DotsAndBoxes)) + "|" +
                              QString::fromStdString(currentUser.getUsername()) + "|" +
                              QString::number(m_p1Score) + "|" +
                              (m_p1Score > m_p2Score ? "Win" : (m_p1Score < m_p2Score ? "Loss" : "Draw")) + "|" +
                              m_opponentUsername + "|" +
                              QString::number(m_p2Score) + "|" +
                              (m_p2Score > m_p1Score ? "Win" : (m_p2Score < m_p1Score ? "Loss" : "Draw"));

            NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);
        }

        QMessageBox::information(this, "Game Over", "Game finished!\nResult: " + winnerText);

        cleanupNetworkAndServer();
        ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    }
}

void DotsAndBoxesWindow::onGameOverReceived(QString message)
{
    if (m_isGameOver) return;

    int totalBoxes = (m_boardSize - 1) * (m_boardSize - 1);
    if (m_p1Score + m_p2Score == totalBoxes) return;

    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    QStringList parts = message.split('|');
    int earnedScore = 0;
    QString myResult = "Win (Opponent Surrendered)";

    if (parts.size() >= 7) {
        if (parts[1] == QString::fromStdString(currentUser.getUsername())) {
            earnedScore = parts[2].toInt();
            myResult = parts[3];
        } else if (parts[4] == QString::fromStdString(currentUser.getUsername())) {
            earnedScore = parts[5].toInt();
            myResult = parts[6];
        }
    }

    GameRecord newRecord;
    newRecord.gameName = GameType::DotsAndBoxes;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = myResult.toStdString();
    newRecord.score = earnedScore;

    if(earnedScore > 0) currentUser.updateScore(GameType::DotsAndBoxes, earnedScore);
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getDotsAndBoxesScore()));
    setupDashboardUI();

    QMessageBox::information(this, "Game Over", QString("The opponent surrendered!\nYou win and earned %1 points!").arg(earnedScore));

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void DotsAndBoxesWindow::on_btn_back_to_dashboard_clicked()
{
    if (m_isGameOver) return;

    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    int maxBoxes = (m_boardSize - 1) * (m_boardSize - 1);

    QString myResult = "Loss (Surrendered)";
    QString oppResult = "Win (Opponent Surrendered)";

    int myEarnedScore = 0;
    int oppEarnedScore = maxBoxes;

    QString payload = QString::number(static_cast<int>(GameType::DotsAndBoxes)) + "|" +
                      QString::fromStdString(currentUser.getUsername()) + "|" +
                      QString::number(myEarnedScore) + "|" +
                      myResult + "|" +
                      m_opponentUsername + "|" +
                      QString::number(oppEarnedScore) + "|" +
                      oppResult;

    NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);

    GameRecord newRecord;
    newRecord.gameName = GameType::DotsAndBoxes;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = myResult.toStdString();
    newRecord.score = myEarnedScore;
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getDotsAndBoxesScore()));
    setupDashboardUI();

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void DotsAndBoxesWindow::onConnectionError(QString errorMsg)
{
    if (m_isGameOver) return;
    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;

    QMessageBox::warning(this, "Connection Error", "Cannot connect to the Room! Please check the IP and Port.");

    ui->btn_create_room_dots_and_boxes->setText("Create Room");
    ui->btn_create_room_dots_and_boxes->setEnabled(true);
    ui->btn_join_room_ip_dots_and_boxes->setText("Join Room");
    ui->btn_join_room_ip_dots_and_boxes->setEnabled(true);
}

void DotsAndBoxesWindow::onErrorReceived(QString errorMsg)
{
    if (m_isGameOver) return;

    m_pendingCreateRoom = false;
    m_pendingJoinRoom = false;

    QMessageBox::warning(this, "Network Error", errorMsg);

    if (ui->stackedWidget->currentWidget() == ui->page_2_gameplay) {
        cleanupNetworkAndServer();
        ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    } else {
        ui->btn_create_room_dots_and_boxes->setText("Create Room");
        ui->btn_create_room_dots_and_boxes->setEnabled(true);
        ui->btn_join_room_ip_dots_and_boxes->setText("Join Room");
        ui->btn_join_room_ip_dots_and_boxes->setEnabled(true);
    }
}
