#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include "manageddockwidget.h"
#include "dockmanager.h"
#include "vrokmain.h"
#include <QDockWidget>

namespace Ui {
class ControlsWidget;
}

class ControlsWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit ControlsWidget(DockManager *manager, VrokMain *vrokMain, QWidget *parent = 0);
    void registerUi();
    ~ControlsWidget();

private slots:
    void on_sbPosition_sliderReleased();

    void on_btPlay_clicked();

    void on_btAbout_clicked();
    void positionTick();
    void on_btPlugins_clicked();

private:
    QTimer tpos;
    Ui::ControlsWidget *ui;
    VrokMain *main;
};

#endif // CONTROLSWIDGET_H
