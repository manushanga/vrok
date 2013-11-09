#ifndef VSWIDGET_H
#define VSWIDGET_H

#include "manageddockwidget.h"

namespace Ui {
class VSWidget;
}

class VSWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit VSWidget(DockManager *manager, VPEffectPlugin *vis, QWidget *parent = 0);
    ~VSWidget();

private:
    Ui::VSWidget *ui;
};

#endif // VSWIDGET_H
