#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include <QGridLayout>
#include <QLineEdit>

#include <QFileDialog>
#include <QMessageBox>
#include <cmath>

#include <QDebug>
#define STEP_COUNT 400

//-------------|
//   private   |
//-------------|
void MainWindow::setMenu() {
    QMenu *mFile = new QMenu("&Файл");
    mFile->addAction("&Создать кривую", this, SLOT(slot_createCurve()));
    mFile->addAction("&Открыть кривую", this, SLOT(slot_openCurve()));
    mFile->addAction("Сохранить &все", this, SLOT(slot_saveAllCurves()));
    mFile->addSeparator();
    mFile->addAction("&Выйти", this, SLOT(close()));

    QMenu *mOptions = new QMenu("&Опции");
    mOptions->addAction("&Задать область отображения",
                        this, SLOT(slot_specifyArea()));
    mOptions->addAction("&Мультиотображение", this, SLOT(slot_changeSelection()));

    QMenu * mHelp = new QMenu("&?");
    mHelp->addAction("&Автор", this, SLOT(slot_author()));
    mHelp->addAction("&О программе", this, SLOT(slot_help()));

    menuBar()->addMenu(mFile);
    menuBar()->addMenu(mOptions);
    menuBar()->addMenu(mHelp);
}

void MainWindow::setWidgets() {

    QWidget *w = new QWidget;
    QHBoxLayout *hLay = new QHBoxLayout;
        //ProjectsView *view = new ProjectsView;
        ProjectsView *view = new ProjectsView;
        QVBoxLayout *vLay = new QVBoxLayout;
            editCurve = new QLineEdit;
            ChartView *chartView = new ChartView;
                chart = new QChart;
                    axis_x = new QCategoryAxis;
                    axis_y = new QCategoryAxis;
            invariantsView = new QTableView;
                invariantsModel = new QStandardItemModel;

    invariantsView->setMaximumHeight(100);
    invariantsView->setMinimumHeight(100);



    setCentralWidget(w);
    w->setLayout(hLay);
        hLay->addWidget(view, 1);
        hLay->addLayout(vLay, 4);
            vLay->addWidget(editCurve);
            vLay->addWidget(chartView);
                chartView->setChart(chart);
                    chart->setAxisX(axis_x);
                    chart->setAxisY(axis_y);
            vLay->addWidget(invariantsView);
                invariantsView->setModel(invariantsModel);

    chartView->setRenderHint(QPainter::Antialiasing);

    connect(editCurve, SIGNAL(returnPressed()), SLOT(slot_returnPressed()));

    model = static_cast<ProjectsModel*>(view->model());
    connect(model, SIGNAL(curveColorChanged(spCurve)),
            SLOT(slot_curveColorChanged(spCurve)));
    //chartView->setRenderHint(QPainter::Antialiasing);
    //chart->setAnimationOptions(QChart::AllAnimations);

    multiSelection = model->getMultiSelection();

    chart->legend()->hide();
    axis_x->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axis_x->setRange(-10, 10);
    axis_x->setStartValue(0);
    axis_y->setRange(-10, 10);
    axis_y->setStartValue(0);
    axis_y->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    slot_refreshAxises();
    connect(axis_x, SIGNAL(rangeChanged(qreal, qreal)), SLOT(slot_refreshAxises()));
    connect(axis_y, SIGNAL(rangeChanged(qreal, qreal)), SLOT(slot_refreshAxises()));


    //axis_x->applyNiceNumbers()

    // инварианты
    connect(chartView, SIGNAL(releaseScroll(qreal, qreal)), SLOT(slot_releaseScroll(qreal, qreal)));
    invariantsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    invariantsModel->setRowCount(7);
    invariantsModel->setColumnCount(2);
    invariantsModel->setHeaderData(0, Qt::Horizontal, "Описание", Qt::DisplayRole);
    invariantsModel->setHeaderData(1, Qt::Horizontal, "Значение", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(0, 0), "Определитель матрицы коэффициентов (Δ)", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(1, 0), "Минор матрицы коэффициентов при x², y², xy (D)", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(2, 0), "Сумма коэффициентов при x² и y² (I)", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(3, 0), "Инвариант относительно поворота системы координат (B)", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(4, 0), "Вырожденная/невырожденная", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(5, 0), "Тип кривой", Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(6, 0), "Канонический вид", Qt::DisplayRole);
    //invariantsModel->setData(invariantsModel->index(7, 0), "Угол поворота", Qt::DisplayRole);
    //invariantsModel->setData(invariantsModel->index(8, 0), "Сдвиг", Qt::DisplayRole);
    invariantsView->resizeColumnsToContents();
    invariantsView->resizeRowsToContents();

}

QVector<spQLineSeries> MainWindow::calcSeriesCurve(spCurve pCurve) {
    QVector<spQLineSeries> result;
    if (!pCurve->isValid())
        return result;

    if (pCurve->getTypeFunction() == Curve::ct_y_by_x) {
        QVector<spQLineSeries> tm_series;
        QVector<double> breakPoints = pCurve->getBreakPoints();
        QVector<QPointF> connPoints = pCurve->getConnectionPoints();
        // сортировка по увеличению
        std::sort(connPoints.begin(), connPoints.end(), [](const QPointF &p1, const QPointF &p2) -> bool {
            return p1.x() < p2.x();
        });
        std::sort(breakPoints.begin(), breakPoints.end(), [](double v1, double v2) {
            return v1 < v2;
        });
        // выставление итераторов для проверки
        auto it_conn = connPoints.begin();
        auto it_break = breakPoints.begin();
        // пропуск точек вне диапазона
        while ((it_conn != connPoints.end()) && (it_conn->x() <  axis_x->min()))
            ++it_conn;
        while ((it_break != breakPoints.end()) && (*it_break < axis_x->min()))
            ++it_break;

//        qDebug() << "conn p count:" << connPoints.size();
//        for (int i = 0; i < connPoints.size(); i++) {
//            qDebug() << "   x =" << connPoints[i].x();
//        }

        double step = (axis_x->max() - axis_x->min()) / STEP_COUNT;
        for (double i = axis_x->min(); i <= axis_x->max(); i += step) {
            QVector<double> values = pCurve->calc_func(i);
            for (int count = tm_series.size(); count < values.size(); count++)
                tm_series.append(spQLineSeries(new QLineSeries));
            for (int j = 0; j < values.size(); j++)
                *tm_series[j] << QPointF(i, values[j]);

            // проверка точки соединения
            while ((it_conn != connPoints.end()) && (it_conn->x() >= i) && (it_conn->x() <= i + step)) {

                if (tm_series.size() == 0) {
                    tm_series.append(spQLineSeries(new QLineSeries));
                    tm_series.append(spQLineSeries(new QLineSeries));
                    *tm_series[0] << *it_conn;
                    *tm_series[1] << *it_conn;
                }
                else {
                    Q_ASSERT(tm_series.size() == 2);
                    *tm_series[0] << *it_conn;
                    *tm_series[1] << *it_conn;
                    result.append(tm_series[0]);
                    result.append(tm_series[1]);
                    tm_series.clear();
                }
                ++it_conn;
            }
            // по точкам разрыва
            while ((it_break != breakPoints.end()) && (*it_break >= i) && (*it_break <= i + step)) {
                if (tm_series.size() != 0) {
                    for (int j = 0; j < tm_series.size(); ++j)
                        result.append(tm_series[j]);
                    tm_series.clear();
                }
                ++it_break;
            }
        }
        // добавление оставшихся
        for (auto it = tm_series.begin(); it != tm_series.end(); ++it)
            result.append(*it);

    }
    else {
        QVector<double> values = pCurve->calc_func(0);
        for (int i = 0; i < values.size(); ++i) {
            QLineSeries *series = new QLineSeries;
            *series << QPointF(values[i], axis_y->min()) << QPointF(values[i], axis_y->max());
            result.append(spQLineSeries(series));
        }
    }

    if (result.size() == 0)
        result.append(spQLineSeries(new QLineSeries));
    return result;
}

void MainWindow::repaintCurves() {
    QVector<spCurve> allCurves;
    for (auto it = selectedCurves.begin(); it != selectedCurves.end(); ++it) {
        allCurves.append(it.key());
        // удаление старых series
        for (auto it_s = it.value().begin(); it_s != it.value().end(); ++it_s)
            chart->removeSeries(it_s->data());
    }
    selectedCurves.clear();

    // вычисление, добавление
    for (auto it = allCurves.begin(); it != allCurves.end(); ++it) {
        QVector<spQLineSeries> resSeries = calcSeriesCurve(*it);
        //qDebug() << resSeries.size();
        for (auto it_s = resSeries.begin(); it_s != resSeries.end(); ++it_s) {
            //(*it_s)->setColor((*it)->getColor());
            (*it_s)->setPen(QPen(QBrush((*it)->getColor()), 2.5));
            chart->addSeries(it_s->data());
            chart->setAxisX(axis_x, it_s->data());
            chart->setAxisY(axis_y, it_s->data());
        }
        selectedCurves.insert(*it, resSeries);
    }
}

void MainWindow::fillInvariants(spCurve pCurve) {
    invariantsModel->setData(invariantsModel->index(0, 1),
                             QString::number(pCurve->getDet()), Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(1, 1),
                             QString::number(pCurve->getD()), Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(2, 1),
                             QString::number(pCurve->getI()), Qt::DisplayRole);
    invariantsModel->setData(invariantsModel->index(3, 1),
                             QString::number(pCurve->getB()), Qt::DisplayRole);
    if (pCurve->getDegeneracy()) {
        invariantsModel->setData(invariantsModel->index(4, 1),
                                 "Вырожденная", Qt::DisplayRole);
    }
    else {
        invariantsModel->setData(invariantsModel->index(4, 1),
                                 "Невырожденная", Qt::DisplayRole);
    }
    invariantsModel->setData(invariantsModel->index(5, 1),
                             pCurve->getTypeCurve(), Qt::DisplayRole);
    // TODO skipped type curve
    invariantsModel->setData(invariantsModel->index(6, 1),
                             pCurve->getCanonicalView(), Qt::DisplayRole);
//    invariantsModel->setData(invariantsModel->index(7, 1),
//                             QString::number(pCurve->getRotateAngle()),
//                             Qt::DisplayRole);

    invariantsView->resizeColumnsToContents();
}

//------------|
//   public   |
//------------|
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMenu();
    setWidgets();

    connect(model, SIGNAL(selectionChanged(spCurve, bool)),
            SLOT(slot_selectionChanged(spCurve, bool)));



}

MainWindow::~MainWindow()
{

}


//-------------------|
//   private slots   |
//-------------------|
#define STEP_AXIS 10
double getStep(double min, double max) {
    double step = (max - min) / STEP_AXIS;
    int degree_10 = 0;
    while (step < 1) {
        step *= 10;
        degree_10--;
    }
    while (step >= 10) {
        step /= 10;
        degree_10++;
    }
    if ((step >= 1) && (step <= 1.5)) {
        step = 1;
    }
    else if ((step > 1.5) && (step <= 3.5)) {
        step = 2;
    }
    else if ((step > 3.5) && (step <= 7.5)) {
        step = 5;
    }
    else {
        step = 10;
    }
    step *= pow(10, degree_10);
    return step;
}

void MainWindow::slot_refreshAxises() {
    // очистка старых labels
    QStringList labels_x = axis_x->categoriesLabels();
    for (auto it = labels_x.begin(); it != labels_x.end(); ++it)
        axis_x->remove(*it);
    QStringList labels_y = axis_y->categoriesLabels();
    for (auto it = labels_y.begin(); it != labels_y.end(); ++it)
        axis_y->remove(*it);
    // поиск шага и заполнение для X
    double step = getStep(axis_x->min(), axis_x->max());
    int start_step = axis_x->min() / step;
    if (start_step > 0) start_step++;
    for (double i = start_step * step; i <= axis_x->max(); i += step, start_step++) {

        if (start_step == 0)
            axis_x->append("0", 0);
        else
            axis_x->append(QString::number(i), i);
    }
    // поиск шага и заполнение для Y
    step = getStep(axis_y->min(), axis_y->max());
    start_step = axis_y->min() / step;
    if (start_step > 0) start_step++;
    for (double i = start_step * step; i <= axis_y->max(); i += step, start_step++) {
        if (start_step == 0)
            axis_y->append("0", 0);
        else
            axis_y->append(QString::number(i), i);
    }


}

void MainWindow::slot_returnPressed() {
    if (currCurve.isNull()) {
        QMessageBox::information(0, "Ошибка", "Кривая не выбрана!");
        return;
    }

    Curve new_curve(editCurve->text(), currCurve->getColor());
    if (!new_curve.isValid()) {
        QMessageBox::information(0, "Ошибка", "Кривая введена неверно!");
        return;
    }

    *currCurve = new_curve;
    editCurve->setText(new_curve.getCurveString());
    repaintCurves();
    fillInvariants(currCurve);

    //qDebug() << "break p:" << currCurve->getBreakPoints();
    //qDebug() << "conn p:" << currCurve->getConnectionPoints();

}

void MainWindow::slot_createCurve() {
    QString filename = QFileDialog::getSaveFileName();
    if (filename.length() == 0)
        return;

    QFile f(filename);
    if (!f.open(QFile::WriteOnly)) {
        QMessageBox::information(0, "Ошибка", "Ошибка создания файла!");
        return;
    }
    QTextStream stream(&f);
    stream << "New curve|";
    f.close();

    if (!model->addCurve(filename)) {
        QMessageBox::information(0, "Ошибка", "Ошибка добавления кривой!");
    }
}

void MainWindow::slot_openCurve() {
    QFileDialog dlg;
    QString filename = dlg.getOpenFileName(nullptr, "Открыть кривую");
    if (filename.length() <= 0)
        return;

    if (!model->addCurve(filename)) {
        QMessageBox::information(nullptr, "Ошибка", "Ошибка добавления кривой!");
    }
}

void MainWindow::slot_saveAllCurves() {
    model->saveAll();
}

void MainWindow::slot_specifyArea() {
    dlg_rect.exec(QVector<double>({ axis_x->min(), axis_x->max(),
                                    axis_y->min(), axis_y->max() }));
    if (dlg_rect.getOk()) {
        QVector<double> result = dlg_rect.getValues();
        Q_ASSERT(result.size() == 4);
        axis_x->setRange(result[0], result[1]);
        axis_y->setRange(result[2], result[3]);
        repaintCurves();
    }
}

void MainWindow::slot_author() {
    QMessageBox::information(0, "Автор", "Студент МАИ\n"
                                         "Группы М8О-213Б-17\n"
                                         "Семенов Сергей Дмитриевич");
}

void MainWindow::slot_help() {
    QMessageBox::information(0, "О программе", "Программа отображает графики кривых второго порядка.");
}

void MainWindow::slot_changeSelection() {
    multiSelection = !multiSelection;
    model->setMultiSelection(multiSelection);
}

void MainWindow::slot_selectionChanged(spCurve pCurve, bool state) {
    // очистка нижней таблицы
    for (int i = 0; i < 7; i++)
        invariantsModel->setData(invariantsModel->index(i, 1), "", Qt::DisplayRole);

    auto it_search = selectedCurves.find(pCurve);
//    if (!pCurve->isValid())
//        return;

    if (state) {
        currCurve = pCurve;
        editCurve->setText(pCurve->getCurveString());
        if (!pCurve->isValid())
            return;

        // вывод инфо о кривой
        fillInvariants(pCurve);

        if (it_search != selectedCurves.end())
            return;

        //printCurve(pCurve);
        QVector<spQLineSeries> series_vec = calcSeriesCurve(pCurve);
        for (auto it = series_vec.begin(); it != series_vec.end(); ++it) {
            (*it)->setPen(QPen(QBrush(pCurve->getColor()), 2.5));
            chart->addSeries(it->data());
            chart->setAxisX(axis_x, it->data());
            chart->setAxisY(axis_y, it->data());
        }
        selectedCurves.insert(pCurve, series_vec);



    }
    else {
        if (it_search == selectedCurves.end())
            return;
        editCurve->setText("");
        if (pCurve == currCurve)
            currCurve = spCurve();

        auto it_selected = selectedCurves.find(pCurve);
        Q_ASSERT(it_selected != selectedCurves.end());
        for (int i = 0; i < it_selected.value().size(); ++i) {
            chart->removeSeries(it_selected.value()[i].data());
        }
        selectedCurves.remove(it_selected.key());

    }
}

void MainWindow::slot_releaseScroll(qreal x, qreal y) {
    axis_x->setRange(axis_x->min() + x, axis_x->max() + x);
    axis_y->setRange(axis_y->min() + y, axis_y->max() + y);

    repaintCurves();
}

void MainWindow::slot_curveColorChanged(spCurve pCurve) {
    if (selectedCurves.find(pCurve) != selectedCurves.end())
        repaintCurves();
}



