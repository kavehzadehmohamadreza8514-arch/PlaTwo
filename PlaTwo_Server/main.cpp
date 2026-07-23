#include <QCoreApplication>
#include "servermanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ServerManager server;
    if (!server.startServer(12345)) {
        return -1;
    }

    return a.exec();
}
