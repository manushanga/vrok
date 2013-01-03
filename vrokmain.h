/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#ifndef VROKMAIN_H
#define VROKMAIN_H

#include <QMainWindow>


#include "vplayer.h"
#include "effects/eq.h"
#include "effects/vis.h"

#include <QGraphicsScene>
#include <QStringListModel>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>

#define BAR_COUNT 16
namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT
    
public:
        Ui::VrokMain *ui;

    explicit VrokMain(QWidget *parent = 0);
    QStringListModel *fileslist;
    QDir *dir;

    ~VrokMain();

public slots:
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_btnOpen_clicked();
    void on_btnFX_clicked();
    void on_btnOpenDir_clicked();
    void on_lvFiles_doubleClicked(QModelIndex i);
    void process();
private:
    QThread *th_spec;

    VPlayer *vp;
    VPEffectPluginEQ *eq;
    std::thread *th;
    bool visuals;
    QGraphicsScene *gs;
    QGraphicsRectItem *gbars[16];
    VPEffectPluginVis *vis;
    QTimer *tx;
    unsigned vis_counter;
    float bar_vals[VPEffectPluginVis::BARS];
};

#endif // VROKMAIN_H
