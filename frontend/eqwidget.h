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
    QSlider *sliders[19];
    QSlider *target_sliders[18];
    QWidget *empty;
    QProgressBar *levels[18];
    QTimer *tx;

public slots:
    void process();
    void changed(int);
    void target_changed(int);
};

#endif // EQWIDGET_H
