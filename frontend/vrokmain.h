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
#include "../effects/eq.h"


#include <QGraphicsScene>
#include <QStringListModel>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QListView>

#include "eqwidget.h"

namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT

public:
    Ui::VrokMain *ui;

    explicit VrokMain(QWidget *parent = 0);

    ~VrokMain();

public slots:
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_btnEQt_clicked();
    void on_btnSpec_clicked();
    void on_btnEQ_clicked();
    void on_btnOpenDir_clicked();
    void on_lvFiles_doubleClicked(QModelIndex i);
    void on_btnAbout_clicked();
    void process();
private:

    VPlayer *vp;
    VPEffectPluginEQ *eq;
    std::thread *th;
    bool visuals;
    QGraphicsScene *gs;
    QGraphicsRectItem *gbars[BAR_COUNT];
    QTimer *tx;
    unsigned vis_counter;
    float bar_vals[BAR_COUNT];
    EQWidget *ew;
};

#endif // VROKMAIN_H
