#ifndef NINEMENSMORRISWINDOW_H
#define NINEMENSMORRISWINDOW_H

#include <QMainWindow>

namespace Ui {
class NineMensMorrisWindow;
}

class NineMensMorrisWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NineMensMorrisWindow(QWidget *parent = nullptr);
    ~NineMensMorrisWindow();

private:
    Ui::NineMensMorrisWindow *ui;
};

#endif // NINEMENSMORRISWINDOW_H
