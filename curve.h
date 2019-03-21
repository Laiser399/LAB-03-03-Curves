
#ifndef CURVE_H
#define CURVE_H

#include <QString>
#include <QVector>
#include <functional>
#include <QPointF>
#include <QLineSeries>


using std::pair; using std::function;
using QtCharts::QLineSeries;

class Curve
{
private:
    QColor color;

    bool valid;
    QVector<QVector<double> > mtx; // ур-е в матричном виде
    double Det, D, I, B; // инварианты
    double L1, L2; // Собств знач

    bool degeneracy; // вырожденность
    QString typeCurve; // тип Кривой
    QString canonicalView; // канонический вид кривой
    double rotateAngle;

    struct coef_struct {
        double x2, y2, xy, x, y, _1;
    } coef;

    int calc_type;
    QVector<QPointF> connectionPoints; // точки соединения кусков графика
    QVector<double> breakPoints; // точки разрыва графика
    function<QVector<double>(double, const coef_struct &)> calc; // ф-я y(x)
    QVector<double> values_x; // массив значений x (если коэф. при всех y == 0)

    void parse(QString curve);
    void calcInvariants();
    void calcOwnValues();
    void calcTypeCurve();
    void calcCanonicalView();
    void calcRotateAngle();
public:
    enum { ct_y_by_x = 1, ct_x_by_y };

    Curve();
    Curve(QString curve, QColor color = QColor(123, 123, 255, 180));
    void setColor(QColor color);
    QColor getColor();

    int getTypeFunction();
    QVector<QPointF> getConnectionPoints();
    QVector<double> getBreakPoints();
    QVector<double> calc_func(double val) const;
    QString getCurveString();
    bool isValid();
    // invariants
    double getDet();
    double getD();
    double getI();
    double getB();
    // prop
    bool getDegeneracy();
    QString getTypeCurve();
    QString getCanonicalView();
    double getRotateAngle();
};

#endif // CURVE_H


