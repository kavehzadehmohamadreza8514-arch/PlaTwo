#include "mainwindow.h"
#include <QSplashScreen>
#include <QTimer>
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile(":/style/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts(&styleFile);
            QString styleSheet = ts.readAll();
            a.setStyleSheet(styleSheet);
            styleFile.close();
        }

    /*
        QSplashScreen *splash = new QSplashScreen(QPixmap(":/images/logo.png")); // یک لوگو در qrc داشته باش
        splash->show();*/

        MainWindow w;

       /* QTimer::singleShot(1500, splash, SLOT(close()));
        QTimer::singleShot(1500, &w, SLOT(show()));*/
    w.show();
    return a.exec();
}
