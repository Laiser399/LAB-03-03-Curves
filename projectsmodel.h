#ifndef PROJECTSMODEL_H
#define PROJECTSMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QString>
#include <QSharedPointer>
#include "curve.h"
#include <QVector>
#include <QFile>

typedef QSharedPointer<Curve> spCurve;


class ProjectsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    struct Node {
        QString filename, name;
        spCurve pCurve;
        bool selected;

        Node(const QString &_filename, const QString &_name,
             spCurve _pCurve){
            filename = _filename;
            name = _name;
            pCurve = _pCurve;
            selected = false;
        }
    };

private:
    typedef QSharedPointer<Node> spNode;
    QVector<spNode> curves;
    bool multiSelection;
    QModelIndex selectedIndex; // если не multiSelection хранит выбранный индекс


public:
    ProjectsModel();
    bool addCurve(const QString &filename);
    bool removeCurve(const QModelIndex &index);
    bool deleteCurve(const QModelIndex &index);
    bool saveCurve(const QModelIndex &index);
    void saveAll();

    bool setSelection(const QModelIndex &index, bool state);

    bool getMultiSelection();
    void setMultiSelection(bool state);
    void setCurveColor(const QModelIndex &index, const QColor &color);

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
signals:
    void selectionChanged(spCurve pCurve, bool state);
    void curveColorChanged(spCurve pCurve);
};

#endif // PROJECTSMODEL_H
