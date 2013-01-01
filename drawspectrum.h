#ifndef DRAWSPECTRUM_H
#define DRAWSPECTRUM_H

#include "effects/vis.h"
#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsView>

class DrawSpectrum : public QObject
{
    Q_OBJECT
private:
    QGraphicsScene *gs;
    QGraphicsRectItem *gbars[16];
    VPEffectPluginVis *vis;
    QWidget *p;
public:
    float bars[VPEffectPluginVis::BARS];
    explicit DrawSpectrum(VPlayer *v, QWidget *gvparent , QObject *parent=0);
    QGraphicsScene *getScene();
    QGraphicsView *gv;
    bool work;
    ~DrawSpectrum();
signals:
    void finished();
public slots:
    void process();
};

#endif // DRAWSPECTRUM_H
