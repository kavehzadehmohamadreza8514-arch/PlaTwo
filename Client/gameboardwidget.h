#ifndef GAMEBOARDWIDGET_H
#define GAMEBOARDWIDGET_H

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include <QPaintEvent>

class GameBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoardWidget(QWidget *parent = nullptr);

    void setGridSize(int rows, int cols);

signals:
    void lineClicked(int row, int col, bool isHorizontal);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int numRows = 4;
    int numCols = 4;

    QVector<QVector<bool>> hLines;
    QVector<QVector<bool>> vLines;

    const int margin = 40;
    const int dotRadius = 6;
};

#endif // GAMEBOARDWIDGET_H
