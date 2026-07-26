#ifndef NINEMENSBOARDWIDGET_H
#define NINEMENSBOARDWIDGET_H

#include <QWidget>
<<<<<<< HEAD
=======
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QPointF>
#include <vector>
>>>>>>> efd0b59a9cf09aab42d0206f7e8bbf4830b244f5

class NineMensBoardWidget : public QWidget
{
    Q_OBJECT
<<<<<<< HEAD
public:
    explicit NineMensBoardWidget(QWidget *parent = nullptr);

signals:

};

#endif // NINEMENSBOARDWIDGET_H
=======

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
>>>>>>> efd0b59a9cf09aab42d0206f7e8bbf4830b244f5
