#include "fanoronawindow.h"
#include "ui_fanoronawindow.h"
#include <QMessageBox>
#include <QPushButton>
#include "networkmanager.h"
#include "mainmenu.h"
#include <QDateTime>

const int FanoronaWindow::DIRECTIONS[8][2] = {
    {-1,-1}, {-1,0}, {-1,1},
    { 0,-1},         { 0,1},
    { 1,-1}, { 1,0}, { 1,1}
};

FanoronaWindow::FanoronaWindow(const User& user, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FanoronaWindow),
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

    ui->lbl_score->setText(QString::number(currentUser.getFanoronaScore()));

    setupDashboardUI();
    setupNetworkUI();

    m_turnTimer = new QTimer(this);
    connect(m_turnTimer, &QTimer::timeout, this, &FanoronaWindow::onTurnTimerTick);

    connect(ui->board_widget, &FanoronaBoardWidget::positionClicked, this, &FanoronaWindow::onPositionClicked);

    connect(&NetworkManager::instance(), &NetworkManager::roomJoined, this, &FanoronaWindow::onRoomJoined);
    connect(&NetworkManager::instance(), &NetworkManager::gameStarted, this, &FanoronaWindow::onGameStarted);
    connect(&NetworkManager::instance(), &NetworkManager::errorReceived, this, &FanoronaWindow::onErrorReceived);
    connect(&NetworkManager::instance(), &NetworkManager::connectionError, this, &FanoronaWindow::onConnectionError);
    connect(&NetworkManager::instance(), SIGNAL(moveReceived(QString)), this, SLOT(onMoveReceived(QString)));
    connect(&NetworkManager::instance(), SIGNAL(turnChanged()), this, SLOT(onTurnChangedReceived()));
    connect(&NetworkManager::instance(), SIGNAL(gameOverReceived(QString)), this, SLOT(onGameOverReceived(QString)));

    connect(&NetworkManager::instance(), &NetworkManager::connectedToServer, this, &FanoronaWindow::onServerConnected);
}

FanoronaWindow::~FanoronaWindow()
{
    cleanupNetworkAndServer();
    delete ui;
}

void FanoronaWindow::cleanupNetworkAndServer()
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


void FanoronaWindow::initGame(bool isHost, QString opponent, int timeLimit)
{
    m_timeLimit = timeLimit;
    m_opponentUsername = opponent;
    m_myPlayerId = isHost ? 1 : 2;
    m_isMyTurn = isHost;
    m_isGameOver = false;

    m_board.assign(TOTAL_CELLS, 0);
    for (int c = 0; c < COLS; ++c) {
        m_board[rowColToPosition(0, c)] = 1;
        m_board[rowColToPosition(1, c)] = 1;
        m_board[rowColToPosition(3, c)] = 2;
        m_board[rowColToPosition(4, c)] = 2;
    }
    int middleRow[COLS] = { 2, 1, 2, 1, 0, 2, 1, 2, 1 };
    for (int c = 0; c < COLS; ++c) {
        m_board[rowColToPosition(2, c)] = middleRow[c];
    }

    m_p1Count = 22;
    m_p2Count = 22;
    m_waitingForChainCapture = false;
    m_chainPiecePos = -1;
    m_lastDirRow = 0;
    m_lastDirCol = 0;
    m_visitedPositions.clear();
    m_selectedPos = -1;

    ui->btn_end_chain_turn->setVisible(false);
    ui->board_widget->setBoardState(m_board);

    if (m_timeLimit > 0) {
        m_remainingTime = m_timeLimit;
        m_turnTimer->start(1000);
    }

    updateGameUI();
}

void FanoronaWindow::positionToRowCol(int pos, int& r, int& c) const {
    r = pos / COLS;
    c = pos % COLS;
}

int FanoronaWindow::rowColToPosition(int r, int c) const {
    return r * COLS + c;
}

bool FanoronaWindow::isConnected(int from, int to) const {
    int r1, c1, r2, c2;
    positionToRowCol(from, r1, c1);
    positionToRowCol(to, r2, c2);
    int dr = r2 - r1;
    int dc = c2 - c1;
    if (dr < -1 || dr > 1 || dc < -1 || dc > 1) return false;
    if (dr == 0 && dc == 0) return false;
    if (dr != 0 && dc != 0) {
        if ((r1 + c1) % 2 != 0) return false;
    }
    return true;
}

std::vector<int> FanoronaWindow::getCaptureChain(int startPos, int dirRow, int dirCol, int opponent) const {
    std::vector<int> captured;
    int r, c;
    positionToRowCol(startPos, r, c);
    int nr = r + dirRow;
    int nc = c + dirCol;
    while (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
        int npos = rowColToPosition(nr, nc);
        if (m_board[npos] == opponent) {
            captured.push_back(npos);
            nr += dirRow;
            nc += dirCol;
        } else {
            break;
        }
    }
    return captured;
}

bool FanoronaWindow::hasCaptureFromPosition(int pos, int player, int forbiddenDirRow, int forbiddenDirCol, const std::vector<int>& visited) const {
    int opponent = (player == 1) ? 2 : 1;
    int r, c;
    positionToRowCol(pos, r, c);
    for (int i = 0; i < 8; ++i) {
        int dr = DIRECTIONS[i][0];
        int dc = DIRECTIONS[i][1];
        if ((forbiddenDirRow != 0 || forbiddenDirCol != 0) && dr == forbiddenDirRow && dc == forbiddenDirCol) continue;
        int nr = r + dr, nc = c + dc;
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
        int destPos = rowColToPosition(nr, nc);
        if (m_board[destPos] != 0) continue;
        if (!isConnected(pos, destPos)) continue;
        bool alreadyVisited = false;
        for (int v : visited) {
            if (v == destPos) { alreadyVisited = true; break; }
        }
        if (alreadyVisited) continue;
        if (!getCaptureChain(destPos, dr, dc, opponent).empty()) return true;
        if (!getCaptureChain(pos, -dr, -dc, opponent).empty()) return true;
    }
    return false;
}

bool FanoronaWindow::hasAnyCaptureAvailable(int player) const {
    std::vector<int> emptyVisited;
    for (int pos = 0; pos < TOTAL_CELLS; ++pos) {
        if (m_board[pos] == player) {
            if (hasCaptureFromPosition(pos, player, 0, 0, emptyVisited)) return true;
        }
    }
    return false;
}

bool FanoronaWindow::hasAnyLegalMove(int player) const {
    if (hasAnyCaptureAvailable(player)) return true;

    for (int pos = 0; pos < TOTAL_CELLS; ++pos) {
        if (m_board[pos] != player) continue;
        int r, c;
        positionToRowCol(pos, r, c);
        for (int i = 0; i < 8; ++i) {
            int nr = r + DIRECTIONS[i][0];
            int nc = c + DIRECTIONS[i][1];
            if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
            int destPos = rowColToPosition(nr, nc);
            if (m_board[destPos] == 0 && isConnected(pos, destPos)) return true;
        }
    }
    return false;
}

std::vector<int> FanoronaWindow::getValidDestinations(int pos) const {
    std::vector<int> validDests;
    int r, c;
    positionToRowCol(pos, r, c);
    int opponent = (m_myPlayerId == 1) ? 2 : 1;
    bool mustCapture = hasAnyCaptureAvailable(m_myPlayerId);

    for (int i = 0; i < 8; ++i) {
        int dr = DIRECTIONS[i][0];
        int dc = DIRECTIONS[i][1];
        if (m_waitingForChainCapture && dr == m_lastDirRow && dc == m_lastDirCol) continue;

        int nr = r + dr, nc = c + dc;
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
        int destPos = rowColToPosition(nr, nc);

        if (m_board[destPos] != 0 || !isConnected(pos, destPos)) continue;

        if (m_waitingForChainCapture) {
            bool alreadyVisited = false;
            for (int v : m_visitedPositions) {
                if (v == destPos) { alreadyVisited = true; break; }
            }
            if (alreadyVisited) continue;
        }

        bool canApproach = !getCaptureChain(destPos, dr, dc, opponent).empty();
        bool canWithdraw = !getCaptureChain(pos, -dr, -dc, opponent).empty();
        bool isCapture = canApproach || canWithdraw;

        if (m_waitingForChainCapture) {
            if (isCapture) validDests.push_back(destPos);
        } else {
            if (mustCapture) {
                if (isCapture) validDests.push_back(destPos);
            } else {
                validDests.push_back(destPos);
            }
        }
    }
    return validDests;
}

void FanoronaWindow::onPositionClicked(int posIndex)
{
    if (!m_isMyTurn || m_isGameOver) return;

    if (m_board[posIndex] == m_myPlayerId) {
        if (m_waitingForChainCapture) {
            if (posIndex != m_chainPiecePos) return;
        }
        m_selectedPos = posIndex;
        ui->board_widget->setSelectedPosition(m_selectedPos);
        ui->board_widget->setHighlightedPositions(getValidDestinations(m_selectedPos));
        return;
    }

    if (m_selectedPos != -1 && m_board[posIndex] == 0) {
        std::vector<int> validDests = getValidDestinations(m_selectedPos);
        bool isValid = false;
        for (int v : validDests) {
            if (v == posIndex) { isValid = true; break; }
        }

        if (!isValid) return;

        int r1, c1, r2, c2;
        positionToRowCol(m_selectedPos, r1, c1);
        positionToRowCol(posIndex, r2, c2);
        int dr = r2 - r1;
        int dc = c2 - c1;
        int opponent = (m_myPlayerId == 1) ? 2 : 1;

        bool canApproach = !getCaptureChain(posIndex, dr, dc, opponent).empty();
        bool canWithdraw = !getCaptureChain(m_selectedPos, -dr, -dc, opponent).empty();

        char moveType = 'P';
        if (canApproach && canWithdraw) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Capture Type");
            msgBox.setText("Select capture method:");
            msgBox.setStyleSheet("QMessageBox { background-color: #1e283c; color: white; } QPushButton { background-color: #00f0b5; color: #060b26; font-weight: bold; border-radius: 5px; padding: 5px 15px; }");
            QPushButton *approachBtn = msgBox.addButton("Approach", QMessageBox::ActionRole);
            QPushButton *withdrawalBtn = msgBox.addButton("Withdrawal", QMessageBox::ActionRole);
            msgBox.exec();
            moveType = (msgBox.clickedButton() == approachBtn) ? 'A' : 'W';
                    } else if (canApproach) {
                        moveType = 'A';
                    } else if (canWithdraw) {
                        moveType = 'W';
                    }

                    QString moveStr = QString("%1,%2,%3").arg(QString(QChar(moveType))).arg(m_selectedPos).arg(posIndex);
                    NetworkManager::instance().sendPacket(PacketType::MOVE_FANORONA, QString::fromStdString(currentUser.getUsername()), moveStr);

                    applyLocalMove(m_myPlayerId, moveType, m_selectedPos, posIndex);
                }
            }

void FanoronaWindow::applyLocalMove(int player, char type, int from, int to)
{
    if (type == 'E') {
        m_waitingForChainCapture = false;
        m_chainPiecePos = -1;
        m_visitedPositions.clear();
        m_selectedPos = -1;
        ui->board_widget->setSelectedPosition(-1);
        ui->board_widget->setHighlightedPositions({});

        if (player == m_myPlayerId) endTurn();

        updateGameUI();
        checkGameOver();
        return;
    }

    int r1, c1, r2, c2;
    positionToRowCol(from, r1, c1);
    positionToRowCol(to, r2, c2);
    int dr = r2 - r1;
    int dc = c2 - c1;
    int opponent = (player == 1) ? 2 : 1;

    m_board[to] = player;
    m_board[from] = 0;

    if (type == 'P') {
        m_waitingForChainCapture = false;
        m_chainPiecePos = -1;
        m_visitedPositions.clear();
        m_selectedPos = -1;
        ui->board_widget->setSelectedPosition(-1);
        ui->board_widget->setHighlightedPositions({});

        if (player == m_myPlayerId) endTurn();

        updateGameUI();
        checkGameOver();
        return;
    }

    std::vector<int> captured = (type == 'A') ? getCaptureChain(to, dr, dc, opponent) : getCaptureChain(from, -dr, -dc, opponent);
    for (int pos : captured) {
        m_board[pos] = 0;
        if (opponent == 1) m_p1Count--;
        else m_p2Count--;
    }

    if (m_visitedPositions.empty()) m_visitedPositions.push_back(from);
    m_visitedPositions.push_back(to);
    m_lastDirRow = dr;
    m_lastDirCol = dc;

    if (hasCaptureFromPosition(to, player, dr, dc, m_visitedPositions)) {
        m_waitingForChainCapture = true;
        m_chainPiecePos = to;

        if (player == m_myPlayerId) {
            m_selectedPos = to;
            ui->board_widget->setSelectedPosition(m_selectedPos);
            ui->board_widget->setHighlightedPositions(getValidDestinations(m_selectedPos));
        }
    } else {
        m_waitingForChainCapture = false;
        m_chainPiecePos = -1;
        m_visitedPositions.clear();
        m_selectedPos = -1;
        ui->board_widget->setSelectedPosition(-1);
        ui->board_widget->setHighlightedPositions({});

        if (player == m_myPlayerId) endTurn();
    }
    updateGameUI();
    checkGameOver();
}

void FanoronaWindow::on_btn_end_chain_turn_clicked()
{
    if (!m_isMyTurn || !m_waitingForChainCapture || m_isGameOver) return;
    NetworkManager::instance().sendPacket(PacketType::MOVE_FANORONA, QString::fromStdString(currentUser.getUsername()), "E");
    applyLocalMove(m_myPlayerId, 'E', -1, -1);
}

void FanoronaWindow::onMoveReceived(QString moveData)
{
    QStringList parts = moveData.split(",");
    if (parts.isEmpty()) return;

    int oppId = (m_myPlayerId == 1) ? 2 : 1;
    char type = parts[0].at(0).toLatin1();

    if (type == 'E') {
        applyLocalMove(oppId, 'E', -1, -1);
    } else if (parts.size() >= 3) {
        int from = parts[1].toInt();
        int to = parts[2].toInt();
        applyLocalMove(oppId, type, from, to);
    }
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
}

void FanoronaWindow::updateGameUI()
{
    ui->lbl_p1_info->setText(QString("P1 (Host): %1 Pieces").arg(m_p1Count));
    ui->lbl_p2_info->setText(QString("P2 (Guest): %1 Pieces").arg(m_p2Count));

    QString turnText = m_isMyTurn ? "YOUR TURN" : "OPPONENT'S TURN";
    if (m_waitingForChainCapture && m_isMyTurn) turnText = "CHAIN CAPTURE!";
    if (m_timeLimit > 0) turnText += QString("\nTime: %1s").arg(m_remainingTime);

    ui->lbl_turn->setText(turnText);
    ui->lbl_turn->setStyleSheet(m_isMyTurn ? "color: #00ffcc; font-weight: bold; font-size: 18px;" : "color: #ff4d6d; font-weight: bold; font-size: 18px;");

    ui->btn_end_chain_turn->setVisible(m_isMyTurn && m_waitingForChainCapture);
    ui->board_widget->setBoardState(m_board);
}

void FanoronaWindow::endTurn()
{
    m_isMyTurn = false;
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    NetworkManager::instance().sendPacket(PacketType::TURN_CHANGE, QString::fromStdString(currentUser.getUsername()), "");
}

void FanoronaWindow::setupDashboardUI()
{
    ui->frame_score->setStyleSheet("QFrame#frame_score { background-color: rgba(20, 30, 48, 0.45); border: 2px solid #00f0b5; border-radius: 20px; }");
    ui->frame_log->setStyleSheet("QFrame#frame_log { background-color: rgba(20, 30, 48, 0.40); border: 1px solid rgba(20, 202, 214, 0.3); border-radius: 25px; }");

    ui->tbl_history->setColumnCount(5);
    QStringList headers = {"Opponent", "Date", "Role", "Result", "Score"};
    ui->tbl_history->setHorizontalHeaderLabels(headers);

    ui->tbl_history->setStyleSheet(
        "QTableWidget { background-color: transparent; gridline-color: transparent; border: none; color: white; font-size: 14px; }"
        "QHeaderView::section { background-color: transparent; color: #a2a8d3; border: none; font-weight: bold; padding: 5px; }"
        "QTableWidget::item { background-color: rgba(255, 255, 255, 0.08); border-radius: 12px; margin: 4px; padding: 5px; }"
    );

    ui->tbl_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tbl_history->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tbl_history->setShowGrid(false);
    ui->tbl_history->setRowCount(0);

    const std::vector<GameRecord>& history = currentUser.getGameHistory();

    for (const auto& record : history) {
        if (record.gameName == GameType::Fanorona) {
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

void FanoronaWindow::setupNetworkUI()
{
    ui->txt_host_port->setPlaceholderText("ENTER HOST PORT");
    ui->txt_guest_ip->setPlaceholderText("ENTER HOST IP");
    ui->txt_guest_port->setPlaceholderText("ENTER HOST PORT");

    QString baseFrameStyle = "QFrame { background-color: rgba(30, 40, 60, 0.4); border-radius: 25px; }";
    QString lineEditStyle = "QLineEdit { background-color: rgba(0, 0, 0, 0.3); border: 1px solid rgba(255, 255, 255, 0.2); border-radius: 15px; color: white; padding: 10px 15px; }";

    ui->frame_mode_selector->setStyleSheet(baseFrameStyle);
    ui->frame_host_setup->setStyleSheet(baseFrameStyle);
    ui->frame_guest_setup->setStyleSheet(baseFrameStyle);
    ui->txt_host_port->setStyleSheet(lineEditStyle);
    ui->txt_guest_ip->setStyleSheet(lineEditStyle);
    ui->txt_guest_port->setStyleSheet(lineEditStyle);

    ui->lbl_host_ip->setStyleSheet("QLabel { border: 2px solid #00f0b5; border-radius: 15px; color: #00f0b5; padding: 5px; }");

    setHostMode(true);
}

void FanoronaWindow::setHostMode(bool isHost)
{
    QString activeHostStyle = "QFrame#frame_host_setup { background-color: rgba(20, 30, 48, 0.6); border: 3px solid #00f0b5; border-radius: 25px; }";
    QString inactiveStyle = "QFrame#frame_host_setup, QFrame#frame_guest_setup { background-color: rgba(20, 30, 48, 0.2); border: 3px solid rgba(255, 255, 255, 0.1); border-radius: 25px; }";
    QString activeGuestStyle = "QFrame#frame_guest_setup { background-color: rgba(20, 30, 48, 0.6); border: 3px solid #ffde59; border-radius: 25px; }";

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
        ui->btn_create_room->setStyleSheet("QPushButton { background-color: #00f0b5; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }");
    } else {
        ui->frame_host_setup->setStyleSheet(inactiveStyle);
        ui->frame_guest_setup->setStyleSheet(activeGuestStyle);
        ui->frame_host_setup->setEnabled(false);
        ui->frame_guest_setup->setEnabled(true);
        ui->btn_select_host->setStyleSheet("QPushButton { color: gray; background: transparent; border: none; }");
        ui->btn_select_guest->setStyleSheet("QPushButton { color: #ffde59; font-weight: bold; background: transparent; border: none; }");
        ui->btn_join_room->setStyleSheet("QPushButton { background-color: #ffde59; color: #060b26; border-radius: 20px; font-weight: bold; padding: 10px; }");
    }
}

void FanoronaWindow::on_btn_select_host_clicked() { setHostMode(true); }
void FanoronaWindow::on_btn_select_guest_clicked() { setHostMode(false); }

void FanoronaWindow::on_btn_start_new_game_clicked()
{
    setHostMode(true);
    ui->txt_host_port->clear();
    ui->txt_guest_ip->clear();
    ui->txt_guest_port->clear();
    ui->stackedWidget->setCurrentWidget(ui->page_setup);
    displayLocalIP();
}

void FanoronaWindow::on_btn_back_clicked()
{
    cleanupNetworkAndServer();
    MainMenu *parentMenu = qobject_cast<MainMenu*>(this->parentWidget());
    if (parentMenu) {
        parentMenu->updateUserData(currentUser);
        parentMenu->show();
    }
    this->close();
}

void FanoronaWindow::on_chk_time_limit_stateChanged(int arg1)
{
    bool hasTimeLimit = (arg1 == Qt::Checked);
    ui->spin_time_min->setEnabled(hasTimeLimit);
    ui->spin_time_sec->setEnabled(hasTimeLimit);
}

void FanoronaWindow::on_btn_create_room_clicked()
{
    if (ui->txt_host_port->text().isEmpty()) { QMessageBox::warning(this, "Validation Error", "Please enter the Host Port."); return; }
    QString portStr = ui->txt_host_port->text();
    ui->btn_create_room->setText("Starting Server...");
    ui->btn_create_room->setEnabled(false);

    if (m_localServerProcess) { m_localServerProcess->kill(); m_localServerProcess->waitForFinished(); delete m_localServerProcess; }
    m_localServerProcess = new QProcess(this);
    m_localServerProcess->start("PlaTwo_Server.exe", QStringList() << portStr);

    NetworkManager::instance().disconnectFromServer();
    m_pendingCreateRoom = true;
    QTimer::singleShot(500, this, [=]() { NetworkManager::instance().connectToServer("127.0.0.1", portStr.toUShort()); });
}

void FanoronaWindow::on_btn_join_room_clicked()
{
    QString hostIp = ui->txt_guest_ip->text();
    QString hostPort = ui->txt_guest_port->text();
    if (hostIp.isEmpty() || hostPort.isEmpty()) { QMessageBox::warning(this, "Validation Error", "Please fill in both Host IP and Port fields."); return; }

    ui->btn_join_room->setText("Joining...");
    ui->btn_join_room->setEnabled(false);
    NetworkManager::instance().disconnectFromServer();
    m_pendingJoinRoom = true;
    NetworkManager::instance().connectToServer(hostIp, hostPort.toUShort());
}

void FanoronaWindow::onServerConnected()
{
    if (m_pendingCreateRoom) {
        m_pendingCreateRoom = false;
        int totalSeconds = 0;
        if (ui->chk_time_limit->isChecked()) { totalSeconds = (ui->spin_time_min->value() * 60) + ui->spin_time_sec->value(); }
        QString payload = QString::fromStdString(currentUser.getUsername()) + "|0|" + QString::number(totalSeconds);
        ui->btn_create_room->setText("Waiting for Guest...");
        NetworkManager::instance().sendPacket(PacketType::CREATE_ROOM, QString::fromStdString(currentUser.getUsername()), payload);
    }
    else if (m_pendingJoinRoom) {
        m_pendingJoinRoom = false;
        NetworkManager::instance().sendPacket(PacketType::JOIN_ROOM, QString::fromStdString(currentUser.getUsername()), "JOIN_ANY_ROOM");
    }
}

void FanoronaWindow::displayLocalIP()
{
    QString localIP = "Unknown";
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) { localIP = address.toString(); break; }
    }
    ui->lbl_host_ip->setText("YOUR LOCAL IP: " + localIP);
}

void FanoronaWindow::onRoomJoined(QString message) {
    if (!message.contains("Waiting")) {
        QStringList parts = message.split("|");
        if (parts.size() >= 3) { initGame(false, parts[0], parts[2].toInt()); ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay); }
    }
}

void FanoronaWindow::onGameStarted(QString message) {
    QStringList parts = message.split("|");
    if (parts.size() >= 3) { initGame(true, parts[0], parts[2].toInt()); ui->stackedWidget->setCurrentWidget(ui->page_2_gameplay); }
}

void FanoronaWindow::onConnectionError(QString errorMsg) {
    if (m_isGameOver) return;
    m_pendingCreateRoom = false; m_pendingJoinRoom = false;
    QMessageBox::warning(this, "Connection Error", "Cannot connect to the Room! Please check the IP and Port.");
    ui->btn_create_room->setText("Create Room"); ui->btn_create_room->setEnabled(true);
    ui->btn_join_room->setText("Join Room"); ui->btn_join_room->setEnabled(true);
}

void FanoronaWindow::onErrorReceived(QString errorMsg) {
    if (m_isGameOver) return;
    m_pendingCreateRoom = false; m_pendingJoinRoom = false;
    QMessageBox::warning(this, "Network Error", errorMsg);
    if (ui->stackedWidget->currentWidget() == ui->page_2_gameplay) {
        cleanupNetworkAndServer(); ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
    } else {
        ui->btn_create_room->setText("Create Room"); ui->btn_create_room->setEnabled(true);
        ui->btn_join_room->setText("Join Room"); ui->btn_join_room->setEnabled(true);
    }
}

void FanoronaWindow::onTurnTimerTick()
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

void FanoronaWindow::checkGameOver()
{
    if (m_isGameOver) return;

    if (m_p1Count == 0 || m_p2Count == 0 || (!hasAnyLegalMove(1) && !hasAnyLegalMove(2))) {
        bool gameOver = true;
        QString winnerText = "";
        QString myResult = "Draw";

        if (m_p1Count == 0 || (!hasAnyLegalMove(1) && m_myPlayerId == 1)) {
            winnerText = "P2 Wins! (P1 is out of pieces or blocked)";
            myResult = (m_myPlayerId == 2) ? "Win" : "Loss";
        } else if (m_p2Count == 0 || (!hasAnyLegalMove(2) && m_myPlayerId == 2)) {
            winnerText = "P1 Wins! (P2 is out of pieces or blocked)";
            myResult = (m_myPlayerId == 1) ? "Win" : "Loss";
        }

        if (m_turnTimer->isActive()) m_turnTimer->stop();
        m_isGameOver = true;

        int score = (myResult == "Win") ? 100 : 0;

        GameRecord newRecord;
        newRecord.gameName = GameType::Fanorona;
        newRecord.opponent = m_opponentUsername.toStdString();
        newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
        newRecord.result = myResult.toStdString();
        newRecord.score = score;

        if (score > 0) currentUser.updateScore(GameType::Fanorona, score);
        currentUser.addGameRecord(newRecord);

        ui->lbl_score->setText(QString::number(currentUser.getFanoronaScore()));
        setupDashboardUI();

        if (m_myPlayerId == 1) {
            QString payload = QString::number(static_cast<int>(GameType::Fanorona)) + "|" +
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

void FanoronaWindow::onGameOverReceived(QString message)
{
    if (m_isGameOver) return;
    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    GameRecord newRecord;
    newRecord.gameName = GameType::Fanorona;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = "Win (Opponent Surrendered/Timeout)";
    newRecord.score = 100;

    currentUser.updateScore(GameType::Fanorona, 100);
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getFanoronaScore()));
    setupDashboardUI();

    QMessageBox::information(this, "Game Over", "The opponent surrendered or ran out of time!\nYou win and earned 100 points!");

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void FanoronaWindow::on_btn_back_to_dashboard_clicked()
{
    if (m_isGameOver) return;
    if (m_turnTimer->isActive()) m_turnTimer->stop();
    m_isGameOver = true;

    QString payload = QString::number(static_cast<int>(GameType::Fanorona)) + "|" +
                      QString::fromStdString(currentUser.getUsername()) + "|0|Loss (Surrendered)|" +
                      m_opponentUsername + "|100|Win (Opponent Surrendered)";

    NetworkManager::instance().sendPacket(PacketType::GAME_OVER, QString::fromStdString(currentUser.getUsername()), payload);

    GameRecord newRecord;
    newRecord.gameName = GameType::Fanorona;
    newRecord.opponent = m_opponentUsername.toStdString();
    newRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    newRecord.playerRole = (m_myPlayerId == 1) ? "Host" : "Guest";
    newRecord.result = "Loss (Surrendered/Timeout)";
    newRecord.score = 0;
    currentUser.addGameRecord(newRecord);

    ui->lbl_score->setText(QString::number(currentUser.getFanoronaScore()));
    setupDashboardUI();

    cleanupNetworkAndServer();
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void FanoronaWindow::onTurnChangedReceived()
{
    m_isMyTurn = true;
    if (m_timeLimit > 0) m_remainingTime = m_timeLimit;
    updateGameUI();
}








