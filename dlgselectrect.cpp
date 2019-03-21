#include "dlgselectrect.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

DlgSelectRect::DlgSelectRect():
    QDialog()
{
    ok = false;
    for (int i = 0; i < 4; i++)
        edits.append(new QLineEdit);
    res_values.resize(4);

    QGridLayout *lay = new QGridLayout;
        QHBoxLayout *b_lay = new QHBoxLayout;
            QPushButton *ok_b = new QPushButton("Ок");
            QPushButton *cancel_b = new QPushButton("Отмена");


    setLayout(lay);
        lay->addWidget(new QLabel("x:  "), 0, 0, Qt::AlignLeft);
        lay->addWidget(new QLabel("y:  "), 1, 0, Qt::AlignLeft);
        lay->addWidget(new QLabel("от"), 0, 1, Qt::AlignRight);
        lay->addWidget(new QLabel("от"), 1, 1, Qt::AlignRight);
        lay->addWidget(edits[0], 0, 2);
        lay->addWidget(edits[2], 1, 2);
        lay->addWidget(new QLabel("до"), 0, 3, Qt::AlignRight);
        lay->addWidget(new QLabel("до"), 1, 3, Qt::AlignRight);
        lay->addWidget(edits[1], 0, 4);
        lay->addWidget(edits[3], 1, 4);
        lay->addLayout(b_lay, 2, 0, 1, 5, Qt::AlignRight);
            b_lay->addWidget(ok_b);
            b_lay->addWidget(cancel_b);


    lay->setColumnStretch(0, 0);
    lay->setColumnStretch(1, 0);
    lay->setColumnStretch(2, 1);
    lay->setColumnStretch(3, 0);
    lay->setColumnStretch(4, 1);

    connect(ok_b, SIGNAL(clicked()), SLOT(slot_okClicked()));
    connect(cancel_b, SIGNAL(clicked()), SLOT(close()));
}


bool DlgSelectRect::getOk() {
    return ok;
}

QVector<double> DlgSelectRect::getValues() {
    if (!ok)
        return QVector<double>();
    return res_values;
}

void DlgSelectRect::exec(const QVector<double> &init_values) {
    if (init_values.size() == 4) {
        for (int i = 0; i < 4; i++) {
            edits[i]->setText(QString::number(init_values[i]));
        }
    }

    QDialog::exec();
}

void DlgSelectRect::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    ok = false;
}

void DlgSelectRect::slot_okClicked() {
    // TODO check values
    bool ok = false;
    for (int i = 0; i < 4; i++) {
        res_values[i] = edits[i]->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::information(nullptr, "Ошибка", "Ошибка преобразования текста в double!");
            return;
        }
    }
    if ((res_values[0] >= res_values[1]) || (res_values[2] >= res_values[3])) {
        QMessageBox::information(nullptr, "Ошибка", "Введены неверные значения!");
        return;
    }

    this->ok = true;
    close();
}


