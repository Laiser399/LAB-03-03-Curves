#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QChartView>
#include <QValueAxis>
using QtCharts::QChartView; using QtCharts::QValueAxis;


class ChartView : public QChartView
{
    Q_OBJECT
private:
    bool pressed;
    QPointF prev_pos;
public:
    ChartView(QWidget *parent = nullptr);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
signals:
    void releaseScroll(qreal x, qreal y);
};

#endif // CHARTVIEW_H
