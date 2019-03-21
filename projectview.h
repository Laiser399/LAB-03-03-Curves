#ifndef PROJECTVIEW_H
#define PROJECTVIEW_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>
#include "projectsmodel.h"


class ProjectsView : public QTreeView
{
    Q_OBJECT
private:
    QMenu menu_curve;
    QModelIndex indexContextMenu;
    ProjectsModel * curr_model;

public:
    explicit ProjectsView(QWidget *parent = nullptr);
    ~ProjectsView();

private slots:
    void slot_contextMenu(const QPoint &pos);
    //
    void menu_changeColor();
    void menu_rename();
    void menu_save();
    void menu_close();
    void menu_delete();

    // QWidget interface
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // PROJECTVIEW_H
