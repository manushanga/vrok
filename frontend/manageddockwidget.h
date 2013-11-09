#ifndef MANAGEDDOCKWIDGET_H
#define MANAGEDDOCKWIDGET_H
#include <QDockWidget>

#include "vplayer.h"
#include "dockmanager.h"

class ManagedDockWidget : public QDockWidget {
    Q_OBJECT
protected:
    DockManager *dockManager;
    QDockWidget *dockWidget;
public:
    ManagedDockWidget(DockManager *manager, QDockWidget *dock, QWidget *parent = 0)
        : dockManager(manager), dockWidget(dock), QDockWidget(parent)
    { }
    void registerUi()
    { dockManager->registerDockWidget(dockWidget); }
    ~ManagedDockWidget()
    { dockManager->unregisterDockWidget(dockWidget); }
};

#endif // MANAGEDDOCKWIDGET_H
