#include "dotsandboxeswindow.h"
#include "ui_dotsandboxeswindow.h"
#include "mainmenu.h"
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDebug>

DotsAndBoxesWindow::DotsAndBoxesWindow(const User& user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DotsAndBoxesWindow),
    currentUser(user)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);

    ui->lbl_score->setText("امتیاز شما در این بازی: " + QString::number(currentUser.getDotsAndBoxesScore()));

    loadGameHistory();

    connect(ui->board_widget, &GameBoardWidget::lineClicked, this, &DotsAndBoxesWindow::onLineClicked);
}

DotsAndBoxesWindow::~DotsAndBoxesWindow()
{
    delete ui;
}

void DotsAndBoxesWindow::loadGameHistory()
{
    ui->tbl_history->setColumnCount(5);
    QStringList headers = {"حریف", "تاریخ", "نقش", "نتیجه", "امتیاز"};
    ui->tbl_history->setHorizontalHeaderLabels(headers);
    ui->tbl_history->horizontalHeader()->setStretchLastSection(true);

    const auto& history = currentUser.getGameHistory();
    ui->tbl_history->setRowCount(0);

    int row = 0;
    for (const auto& record : history) {
        if (record.gameName == GameType::DotsAndBoxes) {
            ui->tbl_history->insertRow(row);
            ui->tbl_history->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(record.opponent)));
            ui->tbl_history->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.date)));
            ui->tbl_history->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.playerRole)));
            ui->tbl_history->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(record.result)));
            ui->tbl_history->setItem(row, 4, new QTableWidgetItem(QString::number(record.score)));
            row++;
        }
    }
}

void DotsAndBoxesWindow::on_btn_back_clicked()
{
    MainMenu *mainMenu = new MainMenu(currentUser);
    mainMenu->show();
    this->close();
}

void DotsAndBoxesWindow::on_btn_start_game_clicked()
{
    if (ui->stackedWidget->count() > 1) {
        ui->stackedWidget->setCurrentIndex(1); // انتقال به Page 2
    } else {
        QMessageBox::information(this, "راهنمایی", "صفحه دوم (صفحه بازی) هنوز در QStackedWidget اضافه نشده است.");
    }
}

void DotsAndBoxesWindow::on_btn_back_to_dashboard_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_1_dashboard);
}

void DotsAndBoxesWindow::onLineClicked(int row, int col, bool isHorizontal)
{
    QString lineType = isHorizontal ? "افقی" : "عمودی";

    qDebug() << "Line clicked -> Type:" << lineType << "Row:" << row << "Col:" << col;

    // در گام بعدی، این داده‌ها در قالب JSON به سرور فرستاده می‌شوند
}

