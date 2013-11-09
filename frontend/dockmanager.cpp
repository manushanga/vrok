#include "dockmanager.h"
#include "vputils.h"
DockManager::DockManager(QMainWindow *main) :
    mainWin(main)
{

}



bool DockManager::registerDockWidget(QDockWidget *widget)
{
    if (widgets.find(widget) == widgets.end()){

        DBG(widget);
        mainWin->addDockWidget(Qt::BottomDockWidgetArea,widget);
        if (widgets.count() > 0) {
            DBG(*widgets.begin()<<"prev ww");
            mainWin->tabifyDockWidget(*widgets.begin(), widget);
        }
        widgets.insert(widget);
        return true;
    } else {
        return false;
    }
}


bool DockManager::unregisterDockWidget(QDockWidget *widget)
{
    DBG("revmoed"<<widget);
    if (widgets.find(widget) != widgets.end()){
 //       mainWin.removeDockWidget((QDockWidget *)(widget));
        widgets.remove(widget);
        return true;
    } else {
        return false;
    }
}
