#include "fanoronaboardwidget.h"
#include <cmath>

FanoronaBoardWidget::FanoronaBoardWidget(QWidget *parent)
    : QWidget(parent), m_selectedPosition(-1), m_hoverPosition(-1)
{
    m_boardState.assign(TOTAL_CELLS, 0);
    setMouseTracking(true);
    m_colorP1 = QColor("#00f0b5");
    m_colorP2 = QColor("#ff4d6d");
}

void FanoronaBoardWidget::setBoardState(const std::vector<int>& state)
{
    if (state.size() == TOTAL_CELLS) {
        m_boardState = state;
        update();
    }
}

void FanoronaBoardWidget::setSelectedPosition(int posIndex)
{
    m_selectedPosition = posIndex;
    update();
}

void FanoronaBoardWidget::setHighlightedPositions(const std::vector<int>& positions)
{
    m_highlightedPositions = positions;
    update();
}

void FanoronaBoardWidget::setPlayerColors(const QColor& p1Color, const QColor& p2Color)
{
    m_colorP1 = p1Color;
    m_colorP2 = p2Color;
    update();
}

std::vector<QPointF> FanoronaBoardWidget::calculatePositions() const
{
    std::vector<QPointF> points(TOTAL_CELLS);
    float margin = 40.0f;
    float usableWidth = width() - 2 * margin;
    float usableHeight = height() - 2 * margin;

    float cellW = usableWidth / (COLS - 1);
    float cellH = usableHeight / (ROWS - 1);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            float x = margin + c * cellW;
            float y = margin + r * cellH;
            points[r * COLS + c] = QPointF(x, y);
        }
    }
    return points;
}

void FanoronaBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    float margin = 20.0f;
    QRectF bgRect(margin, margin, width() - 2 * margin, height() - 2 * margin);
    painter.setBrush(QColor(255, 255, 255, 15));
    painter.setPen(QPen(QColor(255, 255, 255, 40), 2));
    painter.drawRoundedRect(bgRect, 15, 15);

    auto points = calculatePositions();

    QPen linePen(QColor("#14cad6"), 3);
    painter.setPen(linePen);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS - 1; ++c) {
            painter.drawLine(points[r * COLS + c], points[r * COLS + c + 1]);
        }
    }

    for (int c = 0; c < COLS; ++c) {
        for (int r = 0; r < ROWS - 1; ++r) {
            painter.drawLine(points[r * COLS + c], points[(r + 1) * COLS + c]);
        }
    }

    for (int r = 0; r < ROWS - 1; ++r) {
        for (int c = 0; c < COLS - 1; ++c) {
            if ((r + c) % 2 == 0) {
                painter.drawLine(points[r * COLS + c], points[(r + 1) * COLS + (c + 1)]);
            } else {
                painter.drawLine(points[(r + 1) * COLS + c], points[r * COLS + (c + 1)]);
            }
        }
    }

    for (int pos : m_highlightedPositions) {
        if (pos >= 0 && pos < TOTAL_CELLS) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 222, 89, 120));
            painter.drawEllipse(points[pos], 20, 20);
        }
    }

    for (int i = 0; i < TOTAL_CELLS; ++i) {
        QPointF pt = points[i];
        float radius = 15.0f;

        if (m_boardState[i] == 0) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#a2a8d3"));
            painter.drawEllipse(pt, 5, 5);
        }

        if (i == m_selectedPosition) {
            painter.setPen(QPen(QColor(255, 255, 255, 200), 3));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(pt, radius + 8, radius + 8);
        }

        if (m_boardState[i] == 1) {
            painter.setBrush(m_colorP1);
            painter.setPen(QPen(QColor("#ffffff"), 2));
            painter.drawEllipse(pt, radius, radius);
        } else if (m_boardState[i] == 2) {
            painter.setBrush(m_colorP2);
            painter.setPen(QPen(QColor("#ffffff"), 2));
            painter.drawEllipse(pt, radius, radius);
        }

        if (i == m_hoverPosition && m_boardState[i] == 0) {
            painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(pt, radius + 2, radius + 2);
        }
    }
}

int FanoronaBoardWidget::getHitPosition(const QPointF& clickPos) const
{
    auto points = calculatePositions();
    float clickRadius = 22.0f;

    for (int i = 0; i < TOTAL_CELLS; ++i) {
        float dx = clickPos.x() - points[i].x();
        float dy = clickPos.y() - points[i].y();
        if (std::sqrt(dx * dx + dy * dy) <= clickRadius) {
            return i;
        }
    }
    return -1;
}

void FanoronaBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int hitIndex = getHitPosition(event->localPos());
        if (hitIndex != -1) {
            emit positionClicked(hitIndex);
        }
    }
}

void FanoronaBoardWidget::mouseMoveEvent(QMouseEvent *event)
{
    int hitIndex = getHitPosition(event->localPos());
    if (hitIndex != m_hoverPosition) {
        m_hoverPosition = hitIndex;
        update();
    }
}

void FanoronaBoardWidget::leaveEvent(QEvent *event)
{
    m_hoverPosition = -1;
    update();
    QWidget::leaveEvent(event);
}
