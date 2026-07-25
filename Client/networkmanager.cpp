#include "networkmanager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &NetworkManager::onErrorOccurred);
}

NetworkManager::~NetworkManager()
{
    if (socket->isOpen()) {
        socket->close();
    }
}

void NetworkManager::connectToServer(const QString& ip, quint16 port)
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->connectToHost(ip, port);
    }
}

void NetworkManager::disconnectFromServer()
{
    socket->disconnectFromHost();
}

void NetworkManager::sendPacket(PacketType type, const QString& sender, const QString& data)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QString packetStr = QString::number(static_cast<int>(type)) + "|" + sender + "|" + data + "\n";
        socket->write(packetStr.toUtf8());
        socket->flush();
    }
}

void NetworkManager::onConnected()
{
    emit connectedToServer();
}

void NetworkManager::onDisconnected()
{
    emit disconnectedFromServer();
}

void NetworkManager::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit connectionError(socket->errorString());
}

void NetworkManager::onReadyRead()
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        QString rawData = QString::fromUtf8(line).trimmed();

        if (rawData.isEmpty()) continue;

        QStringList parts = rawData.split('|');
        if (parts.size() >= 3) {
            int typeInt = parts[0].toInt();
            PacketType type = static_cast<PacketType>(typeInt);
            QString sender = parts[1];
            QString payload = parts[2];

            for (int i = 3; i < parts.size(); ++i) {
                payload += "|" + parts[i];
            }

            switch (type) {
                case PacketType::CONNECT_REQ:
                    emit authResponseReceived(true, payload);
                    break;
                case PacketType::ERROR_MSG:
                    if (payload.contains("Failed") || payload.contains("mismatch") || payload.contains("Invalid")) {
                        emit authResponseReceived(false, payload);
                    } else {
                        emit errorReceived(payload);
                    }
                    break;
                case PacketType::ROOM_JOINED:
                    emit roomJoined(payload);
                    break;
                case PacketType::GAME_START:
                    emit gameStarted(payload);
                    break;
                case PacketType::MOVE_DOTS_BOXES:
                    emit moveReceived(payload);
                    break;
                case PacketType::TURN_CHANGE:
                    emit turnChanged();
                    break;
                case PacketType::GAME_OVER:
                    emit gameOverReceived(payload);
                    break;
                default:
                    break;
            }
        }
    }
}
