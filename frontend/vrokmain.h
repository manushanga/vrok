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
#include <QStandardItemModel>
#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QListView>
#include <QMenu>
#include <QList>
#include <QAction>
#include <QActionGroup>

#include "eqwidget.h"
#include "displayticker.h"
#include "folderseeker.h"
namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT

public:
    Ui::VrokMain *ui;

    explicit VrokMain(QWidget *parent = 0);
    static void callback_next(char *next, void *user);
    bool eventFilter(QObject *target, QEvent *event);
    void resizeEvent(QResizeEvent *event);
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
    void fillQueue();
    void on_sbFolderSeek_valueChanged(int value);
    void actionQueueTriggered();
    void actionQueueRemove();
    void actionNewQueue();
    void startFillTimer();
private slots:
    void on_lvQueue_doubleClicked(const QModelIndex &index);
    void on_tbQueues_tabCloseRequested(int index);

    void on_tbQueues_currentChanged(int index);

private:
    void folderSeekSweep(QDir& root);
    // player funcs
    QTimer tcb;
    VPlayer *vp;
    QStringList getExtentionsList();
    // effects
    EQWidget *ew;
    VPEffectPluginEQ *eq;

    // visuals
    DisplayTicker *lblDisplay;
    QTimer tx;
    unsigned vis_counter;
    float bar_vals[BAR_COUNT];
    bool visuals;
    QGraphicsScene *gs;
    QGraphicsRectItem *gbars[BAR_COUNT];
    QGraphicsRectItem *gmbars[BAR_COUNT];
    QGraphicsRectItem *gbbars[BAR_COUNT];
    int lastTab;

    // playlist
    void loadDirFilesModel(QString opendir, QStandardItemModel *model);
    QDir curdir;
    QDir cursweep;
    QStringList dirs;
    QStandardItemModel dirFilesModel;

    QStandardItemModel queueModel;
    QList< QAction * > contextMenuFiles;
    QList< QAction * > contextMenuQueue;
    QList< QAction * > contextMenuTab;
    QActionGroup *queueToggleFillType;
};

#endif // VROKMAIN_H
