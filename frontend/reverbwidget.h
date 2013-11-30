#ifndef REVERBWIDGET_H
#define REVERBWIDGET_H

#include <QDockWidget>

#include "manageddockwidget.h"
#include "../effects/reverb/reverb.h"

namespace Ui {
class ReverbWidget;
}

class ReverbWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit ReverbWidget(DockManager *manager, VPEffectPluginReverb *rb, QWidget *parent = 0);
    ~ReverbWidget();

private slots:
    void on_slReverbs_sliderMoved(int position);

    void on_slAmplitude_sliderMoved(int position);

    void on_slDelay_sliderMoved(int position);

    void on_pbAdd_clicked();

private:
    VPEffectPluginReverb *plugin;
    Ui::ReverbWidget *ui;
};

#endif // REVERBWIDGET_H
