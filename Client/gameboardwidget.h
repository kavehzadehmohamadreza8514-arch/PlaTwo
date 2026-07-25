#ifndef GAMEBOARDWIDGET_H
#define GAMEBOARDWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <vector>

class GameBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameBoardWidget(QWidget *parent = nullptr);

    void setBoardSize(int size);
    void updateBoardState(const std::vector<std::vector<int>>& hLines,
                          const std::vector<std::vector<int>>& vLines,
                          const std::vector<std::vector<int>>& boxOwners);

signals:
    void lineClicked(int row, int col, bool isHorizontal);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_boardSize;

    std::vector<std::vector<int>> m_horizontalLines;
    std::vector<std::vector<int>> m_verticalLines;
    std::vector<std::vector<int>> m_boxes;

    int m_hoverRow;
    int m_hoverCol;
    bool m_hoverIsHorizontal;
    bool m_hasHover;

    QColor colorP1;
    QColor colorP2;
    QColor colorHover;

    void drawNeonLine(QPainter& painter, QPointF p1, QPointF p2, QColor color, int thickness);
    void drawNeonBox(QPainter& painter, QRectF rect, int owner);

    float getSpacing() const;
    QPointF getMargin() const;
    QPointF getDotPosition(int row, int col) const;
};

#endif // GAMEBOARDWIDGET_H
