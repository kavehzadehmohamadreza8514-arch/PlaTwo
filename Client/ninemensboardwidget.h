#ifndef NINEMENSBOARDWIDGET_H
#define NINEMENSBOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QPointF>
#include <vector>

class NineMensBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NineMensBoardWidget(QWidget *parent = nullptr);

    void setBoardState(const std::vector<int>& state);
    void setSelectedPosition(int posIndex);

signals:
    void positionClicked(int posIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    std::vector<int> m_boardState;
    int m_selectedPosition;

    std::vector<QPointF> calculatePositions() const;
    int getHitPosition(const QPointF& clickPos) const;
};

#endif
