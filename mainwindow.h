#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "projectsmodel.h"
#include <QTreeView>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QLineSeries>
#include "curve.h"
#include "chartview.h"
#include "projectview.h"
#include <QStandardItemModel>
#include <QTableView>
#include "dlgselectrect.h"

#include <QCategoryAxis>

using QtCharts::QChart; using QtCharts::QChartView;
using QtCharts::QValueAxis; using QtCharts::QLineSeries;
using QtCharts::QCategoryAxis;

typedef QSharedPointer<QLineSeries> spQLineSeries;


class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    ProjectsModel *model;
    QChart *chart;
    QLineEdit *editCurve; //ввод кривой
    //QValueAxis *axis_x, *axis_y; // коорд оси
    QCategoryAxis *axis_x, *axis_y;
    bool multiSelection; // выбор неск кривых
    QMap<spCurve, QVector<spQLineSeries> > selectedCurves;
    spCurve currCurve;
    //spCurve printedCurve;

    QTableView *invariantsView;
    QStandardItemModel *invariantsModel;

    DlgSelectRect dlg_rect;


    void setMenu();
    void setWidgets();

    QVector<spQLineSeries> calcSeriesCurve(spCurve pCurve);

    void repaintCurves();
    void fillInvariants(spCurve pCurve);


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slot_refreshAxises();

    void slot_returnPressed();

    void slot_createCurve();
    void slot_openCurve();
    void slot_saveAllCurves();

    void slot_specifyArea();

    void slot_author();
    void slot_help();

    void slot_changeSelection();

    void slot_selectionChanged(spCurve pCurve, bool state);
    void slot_releaseScroll(qreal x, qreal y);

    void slot_curveColorChanged(spCurve pCurve);
    //void slot_returnPressed();
};

#endif // MAINWINDOW_H
