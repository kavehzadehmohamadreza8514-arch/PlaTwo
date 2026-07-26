#include "ninemensboardwidget.h"
#include <cmath>

NineMensBoardWidget::NineMensBoardWidget(QWidget *parent)
    : QWidget(parent), m_selectedPosition(-1)
{
    m_boardState.assign(24, 0);
}

void NineMensBoardWidget::setBoardState(const std::vector<int>& state)
{
    if (state.size() == 24) {
        m_boardState = state;
        update();
    }
}

void NineMensBoardWidget::setSelectedPosition(int posIndex)
{
    m_selectedPosition = posIndex;
    update();
}

std::vector<QPointF> NineMensBoardWidget::calculatePositions() const
{
    std::vector<QPointF> points(24);

    float w = width();
    float h = height();
    float margin = 30.0f;
    float size = qMin(w, h) - 2 * margin;

    float cx = w / 2.0f;
    float cy = h / 2.0f;

    for (int layer = 0; layer < 3; ++layer) {
        float r = (size / 2.0f) * (1.0f - layer * 0.3f);
        int base = layer * 8;

        points[base + 0] = QPointF(cx - r, cy - r);
        points[base + 1] = QPointF(cx, cy - r);
        points[base + 2] = QPointF(cx + r, cy - r);
        points[base + 3] = QPointF(cx + r, cy);
        points[base + 4] = QPointF(cx + r, cy + r);
        points[base + 5] = QPointF(cx, cy + r);
        points[base + 6] = QPointF(cx - r, cy + r);
        points[base + 7] = QPointF(cx - r, cy);
    }

    return points;
}

void NineMensBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    float margin = 30.0f;
    float size = qMin(width(), height()) - 2 * margin;
    float cx = width() / 2.0f;
    float cy = height() / 2.0f;

    QRectF bgRect(cx - (size / 2.0f) - 20, cy - (size / 2.0f) - 20, size + 40, size + 40);

    painter.setBrush(QColor(255, 255, 255, 30));
    painter.setPen(QPen(QColor(255, 255, 255, 60), 2));
    painter.drawRoundedRect(bgRect, 20, 20);


    auto points = calculatePositions();

    QPen linePen(QColor("#14cad6"), 3);
    painter.setPen(linePen);

    for (int layer = 0; layer < 3; ++layer) {
        int b = layer * 8;
        for (int i = 0; i < 8; ++i) {
            int next = b + ((i + 1) % 8 == 0 ? 0 : i + 1);
            if (i % 2 == 0) {
                painter.drawLine(points[b + i], points[b + i + 1]);
            } else {
                painter.drawLine(points[b + i], points[next]);
            }
        }
    }

    painter.drawLine(points[1], points[17]);
    painter.drawLine(points[3], points[19]);
    painter.drawLine(points[5], points[21]);
    painter.drawLine(points[7], points[23]);

    for (int i = 0; i < 24; ++i) {
        QPointF pt = points[i];
        float radius = 14.0f;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#a2a8d3"));
        painter.drawEllipse(pt, 6, 6);

        if (i == m_selectedPosition) {
            painter.setBrush(QColor(255, 222, 89, 100));
            painter.drawEllipse(pt, radius + 6, radius + 6);
        }

        if (m_boardState[i] == 1) {
            painter.setBrush(QColor("#00f0b5"));
            painter.setPen(QPen(QColor("#ffffff"), 2));
            painter.drawEllipse(pt, radius, radius);
        } else if (m_boardState[i] == 2) {
            painter.setBrush(QColor("#ff4d6d"));
            painter.setPen(QPen(QColor("#ffffff"), 2));
            painter.drawEllipse(pt, radius, radius);
        }
    }
}

int NineMensBoardWidget::getHitPosition(const QPointF& clickPos) const
{
    auto points = calculatePositions();
    float clickRadius = 22.0f;

    for (int i = 0; i < 24; ++i) {
        float dx = clickPos.x() - points[i].x();
        float dy = clickPos.y() - points[i].y();
        if (std::sqrt(dx * dx + dy * dy) <= clickRadius) {
            return i;
        }
    }
    return -1;
}

void NineMensBoardWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int hitIndex = getHitPosition(event->localPos());
        if (hitIndex != -1) {
            emit positionClicked(hitIndex);
        }
    }
}
