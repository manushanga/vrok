#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H
#include <QMainWindow>
#include <QSet>

class ManagedDockWidget;

class DockManager
{

private:
    QMainWindow *mainWin;
    QSet<QDockWidget*> widgets;
public:
    DockManager(QMainWindow *main);
    bool registerDockWidget(QDockWidget *widget);
    bool unregisterDockWidget(QDockWidget *widget);
};

#endif // DOCKMANAGER_H
