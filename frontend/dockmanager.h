#ifndef DOCKMANAGER_H
#define DOCKMANAGER_H
#include <QMainWindow>
#include <QSet>

class ManagedDockWidget;

class DockManager
{
public:
    enum WidgetType { Generic=0, ControlBay, TrackList, Plugin, WidgetTypeEnd };
private:
    QMainWindow *mainWin;
    QList< QSet<QDockWidget*> > widgets;
public:
    DockManager(QMainWindow *main);
    bool registerDockWidget(QDockWidget *widget, WidgetType widgetType);
    bool unregisterDockWidget(QDockWidget *widget);
    ~DockManager();
};

#endif // DOCKMANAGER_H
