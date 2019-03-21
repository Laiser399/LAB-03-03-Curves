#include "projectsmodel.h"
#include <QBrush>
#include <QTextStream>

#include <QDebug>

//#define DEBUG_MODEL

ProjectsModel::ProjectsModel()
{
    multiSelection = false;
}

bool ProjectsModel::addCurve(const QString &filename) {
    QFile fin(filename);


    // search for repeat
    for (auto it = curves.begin(); it != curves.end(); ++it) {
        if ((*it)->filename == fin.fileName())
            return true;
    }

    if (!fin.open(QFile::ReadOnly))
        return false;
    QTextStream stream(&fin);
    QString full_file = stream.readAll();
    fin.close();

    QStringList pair = full_file.split('|');
    if ((pair.size() != 2) || (pair[0].length() <= 0))
        return false;
    spCurve tmp_curve;
    try {
        tmp_curve = spCurve(new Curve(pair[1]));
    } catch (...) {
        return false;
    }
    Q_ASSERT(!tmp_curve.isNull());
    emit beginInsertRows(QModelIndex(), curves.size(), curves.size());
    curves.append(spNode(new Node(fin.fileName(), pair[0], tmp_curve)));
    emit endInsertRows();

    return true;
}

bool ProjectsModel::removeCurve(const QModelIndex &index) {
    if (!index.isValid())
        return false;

    int row = index.row();
    Node *tmp = static_cast<Node*>(index.internalPointer());
    if (tmp->selected) {
        emit selectionChanged(curves[row]->pCurve, false);
        if (!multiSelection)
            selectedIndex = QModelIndex();
    }

    //disconnect(tmp->pCurve.data(), SIGNAL(colorChanged()), SLOT(slot_curveColorChanged()));

    emit beginRemoveRows(QModelIndex(), row, row);
    curves.remove(row);
    emit endRemoveRows();

    return true;
}

bool ProjectsModel::deleteCurve(const QModelIndex &index) {
    if (!index.isValid())
        return false;

    Node *tmp = static_cast<Node*>(index.internalPointer());
    int row = index.row();

    if (!QFile(tmp->filename).remove())
        return false;

    if (tmp->selected) {
        emit selectionChanged(curves[row]->pCurve, false);
        if (!multiSelection)
            selectedIndex = QModelIndex();
    }

    emit beginRemoveRows(QModelIndex(), row, row);
    curves.remove(row);
    emit endRemoveRows();
    return true;
}

bool ProjectsModel::saveCurve(const QModelIndex &index) {
    if (!index.isValid())
        return false;

    Node *tmp = static_cast<Node*>(index.internalPointer());
    QFile fout(tmp->filename);
    if (!fout.open(QFile::WriteOnly))
        return false;
    QTextStream stream(&fout);
    stream << tmp->name << '|' << tmp->pCurve->getCurveString();
    fout.close();
}

void ProjectsModel::saveAll() {
    for (int i = 0; i < curves.size(); i++)
        saveCurve(index(i, 0));
}

bool ProjectsModel::setSelection(const QModelIndex &index, bool state) {
    if (!index.isValid())
        return false;

    Node *tmp = static_cast<Node*>(index.internalPointer());
//    if (tmp->selected == state)
//        return true;
    if (multiSelection) {
        tmp->selected = state;
        emit dataChanged(index, index);
        emit selectionChanged(curves[index.row()]->pCurve, state);
        return true;
    }
    else {
        if (state) {
            if (selectedIndex.isValid()) {
                Node * tmp_prev = static_cast<Node*>(selectedIndex.internalPointer());
                tmp_prev->selected = false;
                emit dataChanged(selectedIndex, selectedIndex);
                emit selectionChanged(curves[selectedIndex.row()]->pCurve, false);
            }
            tmp->selected = true;
            selectedIndex = index;
            emit dataChanged(index, index);
            emit selectionChanged(curves[index.row()]->pCurve, true);
            return true;
        }
        else {
            if (selectedIndex == index) {
                tmp->selected = false;
                selectedIndex = QModelIndex();
                emit dataChanged(index, index);
                emit selectionChanged(curves[index.row()]->pCurve, false);
                return true;
            }
        }
    }
    return false;
}

bool ProjectsModel::getMultiSelection() {
    return multiSelection;
}

void ProjectsModel::setMultiSelection(bool state) {
    if (state == multiSelection)
        return;

    if (state) {
        multiSelection = true;
        selectedIndex = QModelIndex();
        if (curves.size() > 0) {
            emit dataChanged(index(0, 0, QModelIndex()),
                             index(curves.size() - 1, 0, QModelIndex()));

        }
    }
    else {
        multiSelection = false;
        for (int i = 0; i < curves.size(); ++i) {
            curves[i]->selected = false;
            emit selectionChanged(curves[i]->pCurve, false);

        }
        if (curves.size() > 0)
            emit dataChanged(index(0, 0, QModelIndex()),
                             index(curves.size() - 1, 0, QModelIndex()));
    }
}

void ProjectsModel::setCurveColor(const QModelIndex &index, const QColor &color) {
    if (!index.isValid())
        return;

    Node *tmp = static_cast<Node*>(index.internalPointer());
    tmp->pCurve->setColor(color);
    emit dataChanged(index, index, QVector<int>({Qt::BackgroundRole}));
    emit curveColorChanged(tmp->pCurve);
}


// QAbstractItemModel interface
QModelIndex ProjectsModel::index(int row, int column, const QModelIndex &parent) const
{
#ifdef DEBUG_MODEL
    qDebug() << "index";
#endif

    if (parent.isValid())
        return QModelIndex();

    //Q_ASSERT((row >= 0) && (row < curves.size()));
    //Q_ASSERT(column == 0);
    if ((row < 0) || (row >= curves.size()) || (column != 0))
        return QModelIndex();

    return createIndex(row, column, curves[row].data());
}

QModelIndex ProjectsModel::parent(const QModelIndex &child) const
{
#ifdef DEBUG_MODEL
    qDebug() << "parent";
#endif
    return QModelIndex();
}

int ProjectsModel::rowCount(const QModelIndex &parent) const
{
#ifdef DEBUG_MODEL
    qDebug() << "rowCount";
#endif
    if (parent.isValid())
        return 0;
    else
        return curves.size();
}

int ProjectsModel::columnCount(const QModelIndex &parent) const
{
#ifdef DEBUG_MODEL
    qDebug() << "columnCount";
#endif
    Q_UNUSED(parent);
    return 1;
}

QVariant ProjectsModel::data(const QModelIndex &index, int role) const
{
#ifdef DEBUG_MODEL
    qDebug() << "data";
#endif
    if (!index.isValid())
        return QVariant();

    Node *tmp = static_cast<Node*>(index.internalPointer());


    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return tmp->name;
    case Qt::BackgroundRole: {
        if (tmp->selected) {
            return tmp->pCurve->getColor();
        }
        else
            return QBrush(Qt::white);
    }
    case Qt::CheckStateRole: {
        if (!multiSelection)
            return QVariant();
        if (tmp->selected) {
            return Qt::Checked;
        }
        else {
            return Qt::Unchecked;
        }
    }
    default:
        return QVariant();
    }
}

bool ProjectsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
#ifdef DEBUG_MODEL
    qDebug() << "setData";
#endif
    if (!index.isValid())
        return false;


    Node *tmp = static_cast<Node*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        QString new_name(value.toString());
        if ((new_name.length() <= 0) || (new_name.indexOf('|') >= 0))
            return false;
        tmp->name = new_name;
        return true;
    }
    case Qt::BackgroundRole: {
        tmp->pCurve->setColor(value.value<QColor>());
        return true;
    }
    case Qt::CheckStateRole: {
        if (!multiSelection)
            return false;
        if (value == Qt::Checked) {
            tmp->selected = true;
            emit selectionChanged(curves[index.row()]->pCurve, true);
        }
        else {
            tmp->selected = false;
            emit selectionChanged(curves[index.row()]->pCurve, false);
        }
        return true;
    }
    default:
        return false;

    }
}

QVariant ProjectsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
#ifdef DEBUG_MODEL
    qDebug() << "headerData";
#endif
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole) &&
            (section == 0))
        return "Кривые";
    else
        return QVariant();
}

Qt::ItemFlags ProjectsModel::flags(const QModelIndex &index) const
{
#ifdef DEBUG_MODEL
    qDebug() << "flags";
#endif
    if (multiSelection) {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable |
               Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    else {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable |
               Qt::ItemIsEnabled;
    }
}











