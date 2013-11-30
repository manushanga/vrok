#ifndef EQWIDGET_H
#define EQWIDGET_H
#include <QWidget>
#include <QDockWidget>
#include <QSlider>
#include <QProgressBar>
#include <QTimer>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QLabel>

#include "../effects/shibatch/eq.h"

#include "manageddockwidget.h"

namespace Ui {
class EQWidget;
}

class EQWidget : public ManagedDockWidget
{
    Q_OBJECT
    
public:
    explicit EQWidget(DockManager *manager, VPEffectPluginEQ *effectPlugin, QWidget *parent = 0);
    ~EQWidget();
    
private:
    VPEffectPluginEQ *plugin;
    Ui::EQWidget *ui;
    QSlider *target_sliders[19];
    QWidget *empty;
    QLabel labelsBottom[19];
    float preampHistory;
public slots:
    void target_changed(int);
private slots:
    void on_pbReset_clicked();
    void on_pbLoad_clicked();
    void on_pbSave_clicked();
    void on_pbAuto_toggled(bool checked);
};

#endif // EQWIDGET_H
