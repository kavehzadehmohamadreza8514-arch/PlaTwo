#include "gameboardwidget.h"
#include <QPainterPath>
#include <cmath>
#include <QDebug>

GameBoardWidget::GameBoardWidget(QWidget *parent)
    : QWidget(parent),
      m_boardSize(6),
      m_hoverRow(-1),
      m_hoverCol(-1),
      m_hoverIsHorizontal(false),
      m_hasHover(false)
{
    setMouseTracking(true);

    colorP1 = QColor(0, 240, 181);
    colorP2 = QColor(255, 77, 109);
    colorHover = QColor(255, 255, 255, 100);

    setBoardSize(m_boardSize);
}

void GameBoardWidget::setBoardSize(int size)
{
    m_boardSize = size;

    m_horizontalLines.assign(m_boardSize, std::vector<int>(m_boardSize - 1, 0));
    m_verticalLines.assign(m_boardSize - 1, std::vector<int>(m_boardSize, 0));
    m_boxes.assign(m_boardSize - 1, std::vector<int>(m_boardSize - 1, 0));

    update();
}

void GameBoardWidget::updateBoardState(const std::vector<std::vector<int>>& hLines,
                                       const std::vector<std::vector<int>>& vLines,
                                       const std::vector<std::vector<int>>& boxOwners)
{
    m_horizontalLines = hLines;
    m_verticalLines = vLines;
    m_boxes = boxOwners;
    update();
}

float GameBoardWidget::getSpacing() const
{
    int minDimension = std::min(width(), height());
    return minDimension / static_cast<float>(m_boardSize + 1);
}

QPointF GameBoardWidget::getMargin() const
{
    float spacing = getSpacing();
    float totalBoardWidth = (m_boardSize - 1) * spacing;
    float marginX = (width() - totalBoardWidth) / 2.0f;
    float marginY = (height() - totalBoardWidth) / 2.0f;
    return QPointF(marginX, marginY);
}

QPointF GameBoardWidget::getDotPosition(int row, int col) const
{
    QPointF margin = getMargin();
    float spacing = getSpacing();
    return QPointF(margin.x() + col * spacing, margin.y() + row * spacing);
}

void GameBoardWidget::drawNeonLine(QPainter& painter, QPointF p1, QPointF p2, QColor color, int thickness)
{

    painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 50), thickness + 6, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(p1, p2);

    painter.setPen(QPen(QColor(color.red(), color.green(), color.blue(), 120), thickness + 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(p1, p2);

    painter.setPen(QPen(color, thickness, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(p1, p2);
}

void GameBoardWidget::drawNeonBox(QPainter& painter, QRectF rect, int owner)
{
    QColor baseColor = (owner == 1) ? colorP1 : colorP2;
    QString text = (owner == 1) ? "P1" : "P2";

    QColor fill = baseColor;
    fill.setAlpha(60);
    painter.setBrush(fill);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect);

    painter.setPen(baseColor);
    QFont font("Segoe UI", getSpacing() * 0.35, QFont::Bold);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, text);
}

void GameBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // نرم کردن لبه‌ها

    float spacing = getSpacing();
    QPointF margin = getMargin();
    float dotRadius = spacing * 0.12f;
    float lineThickness = spacing * 0.12f;

    QRectF boardRect(margin.x() - spacing*0.8f, margin.y() - spacing*0.8f,
                     (m_boardSize - 1) * spacing + spacing*1.6f,
                     (m_boardSize - 1) * spacing + spacing*1.6f);
    painter.setBrush(QColor(255, 255, 255, 30)); // سفید نیمه‌شفاف
    painter.setPen(QPen(QColor(255, 255, 255, 60), 2));
    painter.drawRoundedRect(boardRect, 20, 20);

    for (int r = 0; r < m_boardSize - 1; ++r) {
        for (int c = 0; c < m_boardSize - 1; ++c) {
            if (m_boxes[r][c] != 0) {
                QRectF boxRect(getDotPosition(r, c).x(), getDotPosition(r, c).y(), spacing, spacing);
                drawNeonBox(painter, boxRect, m_boxes[r][c]);
            }
        }
    }

    for (int r = 0; r < m_boardSize; ++r) {
        for (int c = 0; c < m_boardSize - 1; ++c) {
            if (m_horizontalLines[r][c] != 0) {
                QColor lineColor = (m_horizontalLines[r][c] == 1) ? colorP1 : colorP2;
                drawNeonLine(painter, getDotPosition(r, c), getDotPosition(r, c + 1), lineColor, lineThickness);
            }
        }
    }

    for (int r = 0; r < m_boardSize - 1; ++r) {
        for (int c = 0; c < m_boardSize; ++c) {
            if (m_verticalLines[r][c] != 0) {
                QColor lineColor = (m_verticalLines[r][c] == 1) ? colorP1 : colorP2;
                drawNeonLine(painter, getDotPosition(r, c), getDotPosition(r + 1, c), lineColor, lineThickness);
            }
        }
    }

    if (m_hasHover) {
        QPointF p1, p2;
        if (m_hoverIsHorizontal && m_horizontalLines[m_hoverRow][m_hoverCol] == 0) {
            p1 = getDotPosition(m_hoverRow, m_hoverCol);
            p2 = getDotPosition(m_hoverRow, m_hoverCol + 1);
            drawNeonLine(painter, p1, p2, colorHover, lineThickness);
        } else if (!m_hoverIsHorizontal && m_verticalLines[m_hoverRow][m_hoverCol] == 0) {
            p1 = getDotPosition(m_hoverRow, m_hoverCol);
            p2 = getDotPosition(m_hoverRow + 1, m_hoverCol);
            drawNeonLine(painter, p1, p2, colorHover, lineThickness);
        }
    }

    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    for (int r = 0; r < m_boardSize; ++r) {
        for (int c = 0; c < m_boardSize; ++c) {
            painter.drawEllipse(getDotPosition(r, c), dotRadius, dotRadius);
        }
    }
}

void GameBoardWidget::mouseMoveEvent(QMouseEvent *event)
{
    float spacing = getSpacing();
    float threshold = spacing * 0.3f;
    QPointF pos = event->localPos();

    float minDistance = threshold;
    int bestRow = -1;
    int bestCol = -1;
    bool bestIsHoriz = false;
    bool found = false;

    for (int r = 0; r < m_boardSize; ++r) {
        for (int c = 0; c < m_boardSize - 1; ++c) {
            QPointF p1 = getDotPosition(r, c);
            QPointF p2 = getDotPosition(r, c + 1);

            if (pos.x() >= p1.x() && pos.x() <= p2.x()) {
                float dist = std::abs(pos.y() - p1.y());
                if (dist < minDistance && m_horizontalLines[r][c] == 0) {
                    minDistance = dist;
                    bestRow = r; bestCol = c;
                    bestIsHoriz = true;
                    found = true;
                }
            }
        }
    }

    for (int r = 0; r < m_boardSize - 1; ++r) {
        for (int c = 0; c < m_boardSize; ++c) {
            QPointF p1 = getDotPosition(r, c);
            QPointF p2 = getDotPosition(r + 1, c);

            if (pos.y() >= p1.y() && pos.y() <= p2.y()) {
                float dist = std::abs(pos.x() - p1.x());
                if (dist < minDistance && m_verticalLines[r][c] == 0) {
                    minDistance = dist;
                    bestRow = r; bestCol = c;
                    bestIsHoriz = false;
                    found = true;
                }
            }
        }
    }

    if (found && (m_hoverRow != bestRow || m_hoverCol != bestCol || m_hoverIsHorizontal != bestIsHoriz || !m_hasHover)) {
        m_hoverRow = bestRow;
        m_hoverCol = bestCol;
        m_hoverIsHorizontal = bestIsHoriz;
        m_hasHover = true;
        update();
    } else if (!found && m_hasHover) {
        m_hasHover = false;
        update();
    }
}

void GameBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_hasHover) {
        emit lineClicked(m_hoverRow, m_hoverCol, m_hoverIsHorizontal);

        m_hasHover = false;
        update();
    }
}

void GameBoardWidget::leaveEvent(QEvent *event)
{
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
    QWidget::leaveEvent(event);
}
