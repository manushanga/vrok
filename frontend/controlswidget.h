#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include "events.h"

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
    static void OnStateChangePlaying(void *message, int messageLength, void *user);
    static void OnStateChangePaused(void *message, int messageLength, void *user);

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
    QIcon *play_icon,*pause_icon;
    QTimer tpos;
    Ui::ControlsWidget *ui;
    VrokMain *main;
};

#endif // CONTROLSWIDGET_H
