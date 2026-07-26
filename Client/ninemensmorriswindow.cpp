#include "ninemensmorriswindow.h"
#include "ui_ninemensmorriswindow.h"

NineMensMorrisWindow::NineMensMorrisWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NineMensMorrisWindow)
{
    ui->setupUi(this);
}

NineMensMorrisWindow::~NineMensMorrisWindow()
{
    delete ui;
}
