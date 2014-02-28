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
#include "../effects/shibatch/eq.h"
#include "../effects/visualization/vpeffectvis.h"
#include "../effects/reverb/reverb.h"
#include "../effects/spatial/spatial.h"

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
#include "dockmanager.h"
#include "vswidget.h"
#include "reverbwidget.h"
#include "spatialwidget.h"


namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT
private:
    struct VPEffectInfo {
        QString name;
        VPEffectPlugin *ptr;
        bool enabled;
        VPEffectInfo(QString _name, VPEffectPlugin *_ptr, bool _enabled)
        { name = _name; ptr = _ptr; enabled = _enabled; }
    };
    QList<VPEffectInfo> effects;
    QMap<QString,ManagedDockWidget*> docks;
public:
    Ui::VrokMain *ui;

    explicit VrokMain(QWidget *parent = 0);
    bool eventFilter(QObject *target, QEvent *event);

    ~VrokMain();


    void showAboutVrok();
    void configurePlugins();

    // dock manager
    DockManager *dockManager;
    // player funcs

    QTimer tpos;
    VPlayer *vp;
    // effects
    EQWidget *ew;
    VPEffectPluginEQ *eq;

    // visuals
    VPEffectPluginVis *vz;
    VSWidget *vw;
    //DisplayTicker *lblDisplay;

    // reverb
    VPEffectPluginReverb *rb;
    ReverbWidget *rw;

    // spatial
    VPEffectPluginSpatial *sp;
    SpatialWidget *sw;

    unsigned vis_counter;
    float bar_vals[BAR_COUNT];
    bool visuals;
    QGraphicsRectItem *gbars[BAR_COUNT];
    QGraphicsRectItem *gmbars[BAR_COUNT];
    QGraphicsRectItem *gbbars[BAR_COUNT];
    int lastTab;


};

#endif // VROKMAIN_H
