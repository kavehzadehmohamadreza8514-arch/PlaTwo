#include "gameboardwidget.h"
#include <QPainter>
#include <QColor>

GameBoardWidget::GameBoardWidget(QWidget *parent) : QWidget(parent)
{
    setGridSize(4, 4);
}

void GameBoardWidget::setGridSize(int rows, int cols)
{
    numRows = rows;
    numCols = cols;

    hLines = QVector<QVector<bool>>(numRows, QVector<bool>(numCols - 1, false));

    vLines = QVector<QVector<bool>>(numRows - 1, QVector<bool>(numCols, false));

    update();
}

void GameBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int cellWidth = (width() - 2 * margin) / (numCols - 1);
    int cellHeight = (height() - 2 * margin) / (numRows - 1);

    QPen linePen(QColor("#2C3E50"), 4);
    painter.setPen(linePen);

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols - 1; ++c) {
            if (hLines[r][c]) {
                int x1 = margin + c * cellWidth;
                int y1 = margin + r * cellHeight;
                int x2 = margin + (c + 1) * cellWidth;
                int y2 = y1;
                painter.drawLine(x1, y1, x2, y2);
            }
        }
    }

    for (int r = 0; r < numRows - 1; ++r) {
        for (int c = 0; c < numCols; ++c) {
            if (vLines[r][c]) {
                int x1 = margin + c * cellWidth;
                int y1 = margin + r * cellHeight;
                int x2 = x1;
                int y2 = margin + (r + 1) * cellHeight;
                painter.drawLine(x1, y1, x2, y2);
            }
        }
    }

    painter.setBrush(QColor("#E74C3C"));
    painter.setPen(Qt::NoPen);

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int x = margin + c * cellWidth;
            int y = margin + r * cellHeight;
            painter.drawEllipse(QPoint(x, y), dotRadius, dotRadius);
        }
    }
}

void GameBoardWidget::mousePressEvent(QMouseEvent *event)
{
    int clickX = event->pos().x();
    int clickY = event->pos().y();

    int cellWidth = (width() - 2 * margin) / (numCols - 1);
    int cellHeight = (height() - 2 * margin) / (numRows - 1);

    const int threshold = 15;

    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols - 1; ++c) {
            int x1 = margin + c * cellWidth;
            int x2 = margin + (c + 1) * cellWidth;
            int y = margin + r * cellHeight;

            if (clickX >= x1 && clickX <= x2 && qAbs(clickY - y) <= threshold) {
                if (!hLines[r][c]) {
                    hLines[r][c] = true;
                    update();
                    emit lineClicked(r, c, true);
                }
                return;
            }
        }
    }

    for (int r = 0; r < numRows - 1; ++r) {
        for (int c = 0; c < numCols; ++c) {
            int x = margin + c * cellWidth;
            int y1 = margin + r * cellHeight;
            int y2 = margin + (r + 1) * cellHeight;

            if (clickY >= y1 && clickY <= y2 && qAbs(clickX - x) <= threshold) {
                if (!vLines[r][c]) {
                    vLines[r][c] = true;
                    update();
                    emit lineClicked(r, c, false);
                }
                return;
            }
        }
    }
}
