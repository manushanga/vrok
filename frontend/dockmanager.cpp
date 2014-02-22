#include "dockmanager.h"
#include "vputils.h"
#include "playlistwidget.h"
DockManager::DockManager(QMainWindow *main) :
    mainWin(main)
{
    mainWin->setDockNestingEnabled(true);
    for (int i=0;i<WidgetTypeEnd;i++){
        widgets.append(QSet<QDockWidget*>());
    }
}



bool DockManager::registerDockWidget(QDockWidget *widget, WidgetType widgetType)
{
    if (widgets[widgetType].find(widget) == widgets[widgetType].end()){
        mainWin->addDockWidget(Qt::BottomDockWidgetArea,widget,Qt::Vertical);
        if (widgets[widgetType].count() > 0) {
            mainWin->tabifyDockWidget(*widgets[widgetType].begin(), widget);
        }
        widgets[widgetType].insert(widget);
        return true;
    } else {
        return false;
    }
}


bool DockManager::unregisterDockWidget(QDockWidget *widget)
{
    mainWin->removeDockWidget(widget);
    for (int i=0;i<widgets.count();i++){
        if (widgets[i].remove(widget))
            return true;
    }
    return false;
}

DockManager::~DockManager()
{
    for (int i=0;i<widgets.count();i++){
        foreach (QDockWidget *d, widgets[i]) {
            delete d;
        }

    }
}
