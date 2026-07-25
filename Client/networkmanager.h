#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QStringList>

enum class PacketType {
    CONNECT_REQ = 0,
    CREATE_ROOM = 1,
    JOIN_ROOM = 2,
    ROOM_JOINED = 3,
    ERROR_MSG = 4,
    GAME_START = 5,
    TURN_CHANGE = 6,
    TIME_UP = 7,
    GAME_OVER = 8,
    MOVE_DOTS_BOXES = 9,
    MOVE_NINE_MENS = 10,
    MOVE_FANORONA = 11,
    PAUSE_SAVE_REQ = 12,
    RECONNECT_REQ = 13,
    ROOM_CONFIG_UPDATE = 14
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager& instance() {
        static NetworkManager instance;
        return instance;
    }

    void connectToServer(const QString& ip, quint16 port);
    void disconnectFromServer();
    void sendPacket(PacketType type, const QString& sender, const QString& data);

signals:
    void connectedToServer();
    void disconnectedFromServer();
    void connectionError(QString errorMsg);

    void authResponseReceived(bool isSuccess, QString message);

    void roomJoined(QString message);
    void gameStarted(QString message);
    void errorReceived(QString errorMsg);

    void moveReceived(QString data);
    void turnChanged();
    void gameOverReceived(QString message);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    QTcpSocket *socket;

    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;
};

#endif
