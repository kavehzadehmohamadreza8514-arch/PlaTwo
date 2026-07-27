#ifndef FANORONABOARDWIDGET_H
#define FANORONABOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPointF>
#include <vector>

class FanoronaBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FanoronaBoardWidget(QWidget *parent = nullptr);

    void setBoardState(const std::vector<int>& state);
    void setSelectedPosition(int posIndex);
    void setHighlightedPositions(const std::vector<int>& positions);
    void setPlayerColors(const QColor& p1Color, const QColor& p2Color);

signals:
    void positionClicked(int posIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    static const int ROWS = 5;
    static const int COLS = 9;
    static const int TOTAL_CELLS = 45;

    std::vector<int> m_boardState;
    int m_selectedPosition;
    std::vector<int> m_highlightedPositions;
    int m_hoverPosition;

    QColor m_colorP1;
    QColor m_colorP2;

    std::vector<QPointF> calculatePositions() const;
    int getHitPosition(const QPointF& clickPos) const;
};

#endif
