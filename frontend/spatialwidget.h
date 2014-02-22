#ifndef SPATIALWIDGET_H
#define SPATIALWIDGET_H
#include "manageddockwidget.h"
#include "dockmanager.h"
#include "./effects/spatial/spatial.h"
#include <QDockWidget>
#include <QPixmap>

namespace Ui {
class SpatialWidget;
}

class SpatialWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit SpatialWidget(DockManager *manager, VPEffectPluginSpatial *sp, QWidget *parent = 0);
    void registerUi();

    ~SpatialWidget();

private slots:
    void on_hsL_valueChanged(int value);

    void on_hsH_valueChanged(int value);

    void on_hsD_valueChanged(int value);

private:
    bool initd;
    VPEffectPluginSpatial *plugin;
    Ui::SpatialWidget *ui;
};

#endif // SPATIALWIDGET_H
