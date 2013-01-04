#ifndef EQWIDGET_H
#define EQWIDGET_H

#include <QDockWidget>
#include <QSlider>
#include "effects/eq.h"
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
    QSlider *sliders[19];
public slots:
    void changed(int);
};

#endif // EQWIDGET_H
