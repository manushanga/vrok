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

#include "../effects/eq.h"

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
    QLabel labelsBottom[19];
public slots:
    void target_changed(int);
private slots:
    void on_pbReset_clicked();
    void on_pbLoad_clicked();
    void on_pbSave_clicked();
};

#endif // EQWIDGET_H
