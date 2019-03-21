#include "curve.h"
#include <algorithm>
#include <QTextStream>
#include <cmath>

#include <sstream>

#include <QDebug>



//-------------|
//   private   |
//-------------|
using std::remove_if; using std::stringstream;

struct monom {
    int deg_x, deg_y;
    double coef;
};

monom readMonom(stringstream & stream) {
    monom res;
    res.deg_x = res.deg_y = 0;
    res.coef = 0;

    QChar c;
    c = stream.peek();
    // чтение коэффициента
    if ((c == '-') || (c == '+')) {
        QChar sign = c;
        stream.ignore(1);
        QChar next = stream.peek();
        if ((next.toLower() == 'x') || (next.toLower() == 'y')) {
            res.coef = sign == '-' ? -1 : 1;
        }
        else if ((next != '-') && (next != '+')) {
            stream >> res.coef;
            if (stream.fail())
                throw QString("Curve: readMonom: error reading coef!");
            if (sign == '-') res.coef = -res.coef;
        }
        else
            throw QString("Curve: readMonom: double sign before coefficient!");
    }
    else if ((c.toLower() == 'x') || (c.toLower() == 'y')) {
        res.coef = 1;
    }
    else {
        stream >> res.coef;
        if (stream.fail())
            throw QString("Curve: readMonom: error reading coef!");
        //throw QString("Curve: readMonom: bad symbol while reading coefficient - %1!").arg(c);
    }

    //чтение x, y
    c = stream.peek();
    c = c.toLower();
    while (((c == '*') || (c == 'x') || (c == 'y')) && !stream.eof()) {
        if (c == '*') {
            stream.ignore(1);
            c = stream.peek();
        }

        if ((c == 'x') || (c == 'y')) {
            QChar var = c;
            stream.ignore(1);
            c = stream.peek();
            if (c == '^') {
                stream.ignore(1);
                int degree;
                stream >> degree;
                if (stream.fail()) {
                    throw QString("Curve: readMonom: error reading degree!");
                }
                if (var == 'x')
                    res.deg_x += degree;
                else
                    res.deg_y += degree;
            }
            else {
                if (var == 'x')
                    res.deg_x += 1;
                else
                    res.deg_y += 1;
            }
        }
        else {
            throw QString("Curve: readMonom: bad symbol while reading variable - %1!").arg(c);
        }

        c = stream.peek();
    }
    return res;
}

void Curve::parse(QString curve) {
    auto last_it = remove_if(curve.begin(), curve.end(),
        [](QChar &c) -> bool
            { return c.isSpace(); });
    curve.remove(last_it - curve.begin(), curve.end() - last_it);

    stringstream stream(curve.toStdString());

    QChar c;
    bool after_eq = false; // после =
    while (stream.good() && !stream.eof()) {
        c = stream.peek();
        if (c == '=') {
            after_eq = true;
            stream.ignore(1);
        }
        else {
            monom tm = readMonom(stream);
            if ((tm.deg_x < 0) || (tm.deg_x > 2) || (tm.deg_y < 0) ||
                (tm.deg_y > 2) || (tm.deg_x + tm.deg_y > 2))
                throw QString("Curve: parse: degree of variable is out of range (0 - 2)!");

            if (after_eq)
                tm.coef *= -1;
            if (tm.deg_x == 2) {
                mtx[0][0] += tm.coef;
            }
            else if (tm.deg_y == 2) {
                mtx[1][1] += tm.coef;
            }
            else if ((tm.deg_x == 1) && (tm.deg_y == 1)) {
                mtx[0][1] += tm.coef;
                mtx[1][0] += tm.coef;
            }
            else if (tm.deg_x == 1) {
                mtx[0][2] += tm.coef;
                mtx[2][0] += tm.coef;
            }
            else if (tm.deg_y == 1) {
                mtx[1][2] += tm.coef;
                mtx[2][1] += tm.coef;
            }
            else {
                mtx[2][2] += tm.coef;
            }
        }
    }
    mtx[0][1] /= 2;
    mtx[1][0] /= 2;
    mtx[0][2] /= 2;
    mtx[2][0] /= 2;
    mtx[1][2] /= 2;
    mtx[2][1] /= 2;
}

void Curve::calcInvariants() {
    Det = mtx[0][0] * mtx[1][1] * mtx[2][2] + 2 * mtx[0][1] * mtx[1][2] * mtx[0][2] -
          mtx[0][2] * mtx[1][1] * mtx[0][2] - mtx[1][2] * mtx[1][2] * mtx[0][0] -
          mtx[0][1] * mtx[0][1] * mtx[2][2];
    D = mtx[0][0] * mtx[1][1] - mtx[0][1] * mtx[0][1];
    I = mtx[0][0] + mtx[1][1];
    B = mtx[0][0] * mtx[2][2] - mtx[0][2] * mtx[0][2] +
        mtx[1][1] * mtx[2][2] - mtx[1][2] * mtx[1][2];
}

#define ABS(a) ((a) < 0 ? -(a) : (a))
void Curve::calcOwnValues() {
    double Discr = I * I - 4 * D;
    if (Discr < 0)
        throw QString("Curve: calcOwnValues: error calculatin own values!");
    L1 = (I + sqrt(Discr)) / 2;
    L2 = (I - sqrt(Discr)) / 2;
    // нумерование
    auto swap_d = [](double & v1, double &v2) {
        double tm = v1;
        v1 = v2;
        v2 = tm;
    };
    if (D > 0) {
        // элиптический тип
        if (ABS(L2) < ABS(L1))
            swap_d(L1, L2);
    }
    else if (D < 0) {
        // гиперболический тип
        if (Det != 0) {
            if (L1 * Det <= 0)
                swap_d(L1, L2);
        }
        else {
            if (L1 <= 0)
                swap_d(L1, L2);
        }
    }
    else {
        // параболический тип
        if (L1 != 0)
            swap_d(L1, L2);
    }
}

void Curve::calcTypeCurve() {
    if (Det != 0) {
        if (D != 0) {
            if ((D > 0) && (Det * I < 0))
                typeCurve = "Эллипс";
            else if ((D > 0) && (Det * I > 0))
                typeCurve = "Мнимый эллипс";
            else if (D < 0)
                typeCurve = "Гипербола";
        }
        else {
            typeCurve = "Парабола";
        }
    }
    else {
        if (D > 0)
            typeCurve = "Вещественная точка на пересечении двух прямых";
        else if (D < 0)
            typeCurve = "Пара вещественных пересекающихся прямых";
        else {
            if (B < 0)
                typeCurve = "Пара вещественных параллельных прямых";
            else if (B == 0)
                typeCurve = "Одна вещественная прямая";
            else
                typeCurve = "Пара мнимых параллельных прямых";
        }
    }
}

void Curve::calcCanonicalView() {
    if (D > 0) {
        // эллиптический тип
        if (I * Det < 0) {
            // эллипс
            double a2 = -Det / (L1 * D);
            double b2 = -Det / (L2 * D);
            canonicalView = QString("x^2/") + QString::number(a2) +
                    " + y^2/" + QString::number(b2) + " = 1";
        }
        else if (I * Det > 0) {
            // мнимый эллипс
            double a2 = Det / (L1 * D);
            double b2 = Det / (L2 * D);
            canonicalView = QString("x^2/") + QString::number(a2) +
                    " + y^2/" + QString::number(b2) + " = -1";
        }
        else if (Det == 0) {
            // пара мнимых пересек прямых
            canonicalView = QString("x^2/") + QString::number(1 / ABS(L1)) +
                    " + y^2/" + QString::number(1 / ABS(L2)) + " = 0";
        }
    }
    else if (D < 0) {
        // ниперболический тип
        if (Det != 0) {
            // гипербола
            double a2 = -Det / (L1 * D);
            double b2 = Det / (L2 * D);
            canonicalView = QString("x^2/") + QString::number(a2) +
                    " - y^2/" + QString::number(b2) + " = 1";
        }
        else {
            // пара пересекающихся прямых
            canonicalView = QString("x^2/") + QString::number(1 / L1) +
                    " - y^2/" + QString::number(-1 / L2) + " = 0";
        }
    }
    else {
        if (Det != 0) {
            //парабола
            double p = sqrt(-Det / pow(I, 3));
            canonicalView = QString("y^2 = ") + QString::number(2 * p) + "x";
        }
        else if (B < 0) {
            // пара параллельных прямых
            double b2 = -B / pow(I, 2);
            canonicalView = QString("y^2");
            if (-b2 >= 0)
                canonicalView += " + ";
            canonicalView += QString::number(-b2) + " = 0";
        }
        else if (B > 0) {
            // уравнение пары мнимых параллельных
            double b2 = B / pow(I, 2);
            canonicalView = QString("y^2");
            if (b2 >= 0)
                canonicalView += " + ";
            canonicalView += QString::number(b2) + " = 0";
        }
        else if (B == 0) {
            // пара совпадающих прямых
            canonicalView = "y^2 = 0";
        }
    }
}

void Curve::calcRotateAngle() {
    if ((mtx[0][1] != 0) || (mtx[0][0] != L1)) {
        double cos_f = mtx[0][1] / sqrt(pow(L1 - mtx[0][0], 2) + pow(mtx[0][1], 2));
        double sin_f = (L1 - mtx[0][0]) / sqrt(pow(L1 - mtx[0][0], 2) + pow(mtx[0][1], 2));
        rotateAngle = atan2(sin_f, cos_f);
    }
    else {
        rotateAngle = 0;
    }
}

//------------|
//   public   |
//------------|
Curve::Curve()
{
    valid = false;
}

Curve::Curve(QString curve, QColor color)
{
    this->color = color;

    valid = true;
    if (curve.simplified().length() == 0) {
        valid = false;
        return;
    }

    mtx.resize(3);
    mtx.squeeze();
    for (int i = 0; i < 3; ++i) {
        mtx[i].resize(3);
        mtx[i].squeeze();
    }

    try {
        parse(curve);
        calcInvariants();
        calcOwnValues();
        calcTypeCurve();
        calcCanonicalView();
        calcRotateAngle();
        // вырожденность
        if (Det == 0)
            degeneracy = true;
        else
            degeneracy = false;
    }
    catch (...) {
        valid = false;
        return;
    }

    coef.x2 = mtx[0][0];
    coef.y2 = mtx[1][1];
    coef.xy = 2 * mtx[0][1];
    coef.x = 2 * mtx[0][2];
    coef.y = 2 * mtx[1][2];
    coef._1 = mtx[2][2];

    //
    if (coef.y2 != 0) {
        calc_type = ct_y_by_x;
        calc = [](double val, const coef_struct & coef) -> QVector<double> {
            double D = pow(coef.xy * val + coef.y, 2) -
                    4 * coef.y2 * (coef.x2 * val * val + coef._1 + coef.x * val);
            if (D < 0)
                return {};

            double res1 = (-coef.xy * val - coef.y + sqrt(D)) / (2 * coef.y2);
            double res2 = (-coef.xy * val - coef.y - sqrt(D)) / (2 * coef.y2);
            return { res1, res2 };
        };
        double a = coef.xy * coef.xy - 4 * coef.y2 * coef.x2;
        double b = 2 * coef.xy * coef.y - 4 * coef.y2 * coef.x;
        double c = coef.y * coef.y - 4 * coef.y2 * coef._1;

        if (a == 0) {
            if (b != 0) {
                double x = -c / b;
                QVector<double> y_vec = calc(x, coef);
                if (y_vec.size() > 0)
                    connectionPoints.append(QPointF(x, y_vec[0]));
            }
        }
        else {
            double D = b * b - 4 * a * c;
            if (D > 0) {
                double x1 = (-b + sqrt(D)) / (2 * a);
                double x2 = (-b - sqrt(D)) / (2 * a);
                double y1 = (-coef.xy * x1 - coef.y) / (2 * coef.y2);
                double y2 = (-coef.xy * x2 - coef.y) / (2 * coef.y2);
                connectionPoints.append(QPointF(x1, y1));
                connectionPoints.append(QPointF(x2, y2));
            }
            else if (D == 0) {
                double x = -b / (2 * a);
                double y = (-coef.xy * x - coef.y) / (2 * coef.y2);
                connectionPoints.append(QPointF(x, y));
            }
        }

    }
    else if (coef.y != 0) {
        calc_type = ct_y_by_x;
        calc = [](double val, const coef_struct & coef) -> QVector<double> {
            double tm = coef.xy * val + coef.y;
            if (tm == 0)
                return {};
            double res = (-coef.x2 * val * val - coef.x * val - coef._1) / tm;
            return { res };
        };
        // вычисление точки разрыва
        if (coef.xy != 0) {
            breakPoints.append(-coef.y / coef.xy);
        }
    }
    else if (coef.xy != 0) {
        calc_type = ct_y_by_x;
        calc = [](double val, const coef_struct & coef) -> QVector<double> {
            if (val == 0)
                return {};
            double res = (-coef.x2 * val *val - coef.x * val - coef._1) / (coef.xy * val);
            return { res };
        };
        // точка разрыва
        breakPoints.append(0);
    }
    else if (coef.x2 != 0) {
        double D = coef.x * coef.x - 4 * coef.x2 * coef._1;
        if (D > 0) {
            values_x.append((-coef.x + sqrt(D)) / (2 * coef.x2));
            values_x.append((-coef.x - sqrt(D)) / (2 * coef.x2));
        }
        else if (D == 0) {
            values_x.append(-coef.x / (2 * coef.x2));
        }
        calc_type = ct_x_by_y;
    }
    else if (coef.x != 0) {
        values_x.append(-coef._1 / coef.x);
        calc_type = ct_x_by_y;
    }
    else {
        valid = false;
        return;
    }
    //qDebug() << Det;
    //qDebug() << D;
    //qDebug() << I;


}

void Curve::setColor(QColor n_color) {
    color = n_color;
}

QColor Curve::getColor() {
    return this->color;
}

int Curve::getTypeFunction() {
    return calc_type;
}

QVector<QPointF> Curve::getConnectionPoints() {
    return connectionPoints;
}

QVector<double> Curve::getBreakPoints() {
    return breakPoints;
}

QVector<double> Curve::calc_func(double val) const {
    if (!valid)
        return QVector<double>();

    if (calc_type == ct_y_by_x) {
        return calc(val, coef);
    }
    else {
        return values_x;
    }

}

QString Curve::getCurveString() {
    if (!valid)
        return QString();

//    struct coef_struct {
//        double x2, y2, xy, x, y, _1;
//    } coef;
    QString res;
    double coefs[6] = { coef.x2, coef.y2, coef.xy, coef.x, coef.y, coef._1 };
    QStringList vars = { "x^2", "y^2", "xy", "x", "y", "" };

    for (int i = 0; i < 5; i++) {
        if (coefs[i] == 1) {
            if (res.length() > 0)
                res += "+";
            res += vars[i];
        }
        else if (coefs[i] == -1) {
            res += QString("-") + vars[i];
        }
        else if (coefs[i] > 0) {
            if (res.length() > 0)
                res += "+";
            res += QString::number(coefs[i]) + vars[i];
        }
        else if (coefs[i] < 0) {
            res += QString::number(coefs[i]) + vars[i];
        }
    }
    // отдельно для числа
    if (coefs[5] != 0) {
        if (coefs[5] > 0)
            res += "+";
        res += QString::number(coefs[5]);
    }

    res += "=0";
    return res;
}

bool Curve::isValid() {
    return valid;
}

double Curve::getDet() {
    return Det;
}
double Curve::getD(){
    return D;
}
double Curve::getI(){
    return I;
}
double Curve::getB(){
    return B;
}

bool Curve::getDegeneracy() {
    return degeneracy;
}
QString Curve::getTypeCurve() {
    return typeCurve;
}
QString Curve::getCanonicalView() {
    return canonicalView;
}
double Curve::getRotateAngle() {
    return rotateAngle;
}









