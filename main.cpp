#include "mainwindow.h"
#include <QApplication>

#include "curve.h"

#include <QDebug>

int main(int argc, char *argv[])
{



    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();

    //Curve curve("x^2 + y^2 + 2xy - 2 = 0");
    //Curve curve("x = y^2");

    //Curve curve("y + xy + x^2 = 0");


}
