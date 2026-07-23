#include "servermanager.h"
#include <QDebug>

ServerManager::ServerManager(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &ServerManager::onNewConnection);
}

bool ServerManager::startServer(quint16 port)
{
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server failed to start on port" << port;
        return false;
    }
    qDebug() << "Server running on port" << port;
    return true;
}

void ServerManager::onNewConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();
    clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::disconnected, this, &ServerManager::onClientDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead, this, &ServerManager::onReadyRead);

    qDebug() << "New Client connected:" << clientSocket->peerAddress().toString();
}

void ServerManager::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        qDebug() << "Client disconnected:" << clientSocket->peerAddress().toString();
        clients.removeOne(clientSocket);
        clientSocket->deleteLater();
    }
}

void ServerManager::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    qDebug() << "Data received from client:" << data;
}
