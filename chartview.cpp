

#include "chartview.h"
#include <QDebug>

ChartView::ChartView(QWidget *parent):
    QChartView(parent), pressed(false)
{
}

void ChartView::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        pressed = true;
        prev_pos = chart()->mapToValue(event->localPos());
    }
}

void ChartView::mouseReleaseEvent(QMouseEvent *event) {
    if (pressed && !(event->buttons() & Qt::LeftButton)) {
        pressed = false;

        QPointF new_pos = chart()->mapToValue(event->localPos());
        QPointF shift = prev_pos - new_pos;

        emit releaseScroll(shift.x(), shift.y());



//        QValueAxis *axis_x = static_cast<QValueAxis*>(chart()->axisX());
//        QValueAxis *axis_y = static_cast<QValueAxis*>(chart()->axisY());
//        double x_width = axis_x->max() - axis_x->min();
//        double y_width = axis_y->max() - axis_y->min();

//        QPointF new_pos = chart()->mapToValue(event->localPos());
//        qreal x = chart()->plotArea().width() / x_width *
//                (prev_pos.x() - new_pos.x());
//        qreal y = chart()->plotArea().height() / y_width *
//                (prev_pos.y() - new_pos.y());

        //qDebug() << "plot width:" << chart()->plotArea().width();
        //qDebug() << "x width:" << x_width;
        //qDebug() << "prev x:" << prev_pos.x();
        //qDebug() << "new x:" << new_pos.x();


        //emit releaseScroll(x, y);
    }
}
