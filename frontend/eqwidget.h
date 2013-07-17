#ifndef EQWIDGET_H
#define EQWIDGET_H
#include <QWidget>
#include <QDockWidget>
#include <QSlider>
#include <QProgressBar>
#include "../effects/eq.h"
#include <QTimer>

namespace Ui {
class EQWidget;
}

class EQWidget : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit EQWidget(VPEffectPluginEQ *eq, QWidget *parent = 0);
    ~EQWidget();
    
private:
    Ui::EQWidget *ui;
    VPEffectPluginEQ *plugin;
    QSlider *target_sliders[19];
    QWidget *empty;

public slots:
    void target_changed(int);
};

#endif // EQWIDGET_H
