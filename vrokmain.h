#ifndef VROKMAIN_H
#define VROKMAIN_H

#include <QMainWindow>
/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <vplayer.h>
#include "effects/eq.h"
#include "effects/vis.h"
#include <thread>
#include <QGraphicsScene>

#define BAR_COUNT 16
namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT
    
public:
    float bars[VPEffectPluginVis::BARS];
    static void vis_updater(VrokMain *self);
    explicit VrokMain(QWidget *parent = 0);
    ~VrokMain();

public slots:
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_btnOpen_clicked();
    void on_btnFX_clicked();

private:
    Ui::VrokMain *ui;
    VPlayer *vp;
    VPEffectPluginVis *vis;
    QGraphicsScene *gs;
    QGraphicsRectItem *gbars[16];
    VPEffectPluginEQ *eq;
    std::thread *th;
    bool visuals;
};

#endif // VROKMAIN_H
