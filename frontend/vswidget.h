#ifndef VSWIDGET_H
#define VSWIDGET_H

#include "manageddockwidget.h"
#include "../effects/visualization/vpeffectvis.h"
#include "ooura_fft.h"

#include <QGraphicsScene>
#include <QTimer>
#include <QGradient>

#define VISBUFFER_FRAMES VPBUFFER_PERIOD
namespace Ui {
class VSWidget;
}

class VDisplay : public QWidget
{
    Q_OBJECT

public:

    VDisplay(VPEffectPluginVis *eff, QWidget *parent=0) :
        QWidget(parent),
        plugin(eff),
        ii(new QImage(width(),height(),QImage::Format_ARGB32))
    {
        bars = (float*)ALIGNED_ALLOC(sizeof(float)*VISBUFFER_FRAMES);
        diff = (float*)ALIGNED_ALLOC(sizeof(float)*VISBUFFER_FRAMES);
        diff1 = (float*)ALIGNED_ALLOC(sizeof(float)*VISBUFFER_FRAMES);
        diff2 = (float*)ALIGNED_ALLOC(sizeof(float)*VISBUFFER_FRAMES);
        bins = (float*)ALIGNED_ALLOC(sizeof(float)*VISBUFFER_FRAMES*VISBUFFER_FRAMES);

        ip= new int[int (sqrt(VISBUFFER_FRAMES/2.0f) ) +2];
        ip[0]=0;
        w = new float[VISBUFFER_FRAMES/2];
        for (int i=0;i<VISBUFFER_FRAMES;i++) { bars[i]=0.0f; }

        QLinearGradient g;
        g.setColorAt(1.0,QColor(255,0,0));
        g.setColorAt(0.9,QColor(130,255,0));
        g.setStart(0,height());
        g.setFinalStop(0,0);


        brush = new QBrush(g);
        bars_cumm = new float[VISBUFFER_FRAMES];
        for (int i=0;i<VISBUFFER_FRAMES;i++){ bars_cumm[i]=0.0f; diff[i]=0.0f; diff1[i]=0.0f; diff2[i]=0.0f; bins[i]=0.0f; }
        subbuffers = VPBUFFER_FRAMES / VISBUFFER_FRAMES;
        setBaseSize(100,100);
        ii->fill(QColor(255,255,255));
        setAutoFillBackground(false);
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);

        current=0;
    }
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    ~VDisplay(){ delete brush; delete ii;  ALIGNED_FREE(bars);
                 ALIGNED_FREE(diff);  ALIGNED_FREE(diff1); ALIGNED_FREE(diff2);
                 ALIGNED_FREE(bins); }
signals:
    void doubleClicked();
private:
    int *ip;
    int subbuffers;
    int current;
    QBrush *brush;
    //long *bins;
    float *bars,*bars_cumm,*w,*diff,*diff1,*diff2,*bins;
    QImage *ii;
    VPEffectPluginVis *plugin;
    float padw, padh;
};
class VSWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit VSWidget(DockManager *manager, VPEffectPluginVis *vis, QWidget *parent = 0);
    void registerUi();
    ~VSWidget();
public slots:
    void process();
    void dispDoubleClicked();

private:
    VDisplay disp;
    VPEffectPluginVis *plugin;
    QTimer q;
    Ui::VSWidget *ui;
};

#endif // VSWIDGET_H
