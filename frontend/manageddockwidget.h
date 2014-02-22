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
    virtual void registerUi()
    {
        DBG("Registering as Generic, if you see this you need to override 'registerUi()'");
        dockManager->registerDockWidget(dockWidget, DockManager::Generic);
    }
    virtual void unregisterUi()
    {
        dockManager->unregisterDockWidget(dockWidget);
    }
    ~ManagedDockWidget()
    {
        DBG("destroying "<<dockWidget); unregisterUi();
    }
};

#endif // MANAGEDDOCKWIDGET_H
