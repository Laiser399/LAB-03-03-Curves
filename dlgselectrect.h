#ifndef DLGSELECTRECT_H
#define DLGSELECTRECT_H

//#include <QObject>
//#include <QWidget>
#include <QDialog>
#include <QLineEdit>

class DlgSelectRect : public QDialog
{
    Q_OBJECT
private:
    QVector<QLineEdit*> edits;
    bool ok;
    QVector<double> res_values;


public:
    DlgSelectRect();
    bool getOk();
    QVector<double> getValues();
    void exec(const QVector<double> &init_values = QVector<double>());

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
private slots:
    void slot_okClicked();
};

#endif // DLGSELECTRECT_H
