#include "projectview.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QColorDialog>
#include <QDebug>

typedef ProjectsModel::Node Node;



ProjectsView::ProjectsView(QWidget *parent):
    QTreeView(parent)
{
    curr_model = new ProjectsModel;
    setModel(curr_model);

    menu_curve.addAction("Изменить цвет", this, SLOT(menu_changeColor()));
    menu_curve.addAction("Переименовать", this, SLOT(menu_rename()));
    menu_curve.addAction("Сохранить", this, SLOT(menu_save()));
    menu_curve.addAction("Закрыть", this, SLOT(menu_close()));
    menu_curve.addSeparator();
    menu_curve.addAction("Удалить", this, SLOT(menu_delete()));


    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            SLOT(slot_contextMenu(const QPoint &)));
}

ProjectsView::~ProjectsView() {}

//-------------------|
//   private slots   |
//-------------------|
void ProjectsView::slot_contextMenu(const QPoint &pos) {
    indexContextMenu = indexAt(pos);
    if (!indexContextMenu.isValid())
        return;

    QPoint pos_g = mapToGlobal(pos);
    pos_g.setY(pos_g.y() + 24);
    //Node *tmp = static_cast<Node*>(indexContextMenu.internalPointer());
    menu_curve.exec(pos_g);
}

void ProjectsView::menu_changeColor() {
    if (!indexContextMenu.isValid())
        return;

    Node *tmp = static_cast<Node*>(indexContextMenu.internalPointer());
    QColor new_col = QColorDialog::getColor(tmp->pCurve->getColor());
    if (!new_col.isValid())
        return;

    curr_model->setCurveColor(indexContextMenu, new_col);
}

void ProjectsView::menu_rename() {
    if (!indexContextMenu.isValid())
        return;
    edit(indexContextMenu);
}

void ProjectsView::menu_save() {
    if (!indexContextMenu.isValid())
        return;

    if (!curr_model->saveCurve(indexContextMenu)) {
        QMessageBox::information(nullptr, "Ошибка", "Ошибка сохранения кривой!");
    }
}

void ProjectsView::menu_close() {
    curr_model->removeCurve(indexContextMenu);
}

void ProjectsView::menu_delete() {
    if (!curr_model->deleteCurve(indexContextMenu)) {
        QMessageBox::information(nullptr, "Ошибка", "Ошибка удаления кривой!");
    }
}

void ProjectsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    //Node *tmp_n = static_cast<Node*>(index.internalPointer());

    curr_model->setSelection(index, true);

}






















