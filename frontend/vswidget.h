#ifndef VSWIDGET_H
#define VSWIDGET_H

#include "manageddockwidget.h"
#include "../effects/visualization/vpeffectvis.h"

#include <QGraphicsScene>
#include <QTimer>

namespace Ui {
class VSWidget;
}

class VDisplay : public QWidget
{
    Q_OBJECT

public:

    enum vis_t{SCOPE, SPECTRUM};
    VDisplay(vis_t vis_type, float *data,QWidget *parent=0) :
        QWidget(parent),
        bars(data),
        type(vis_type),
        ii(new QImage(width(),height(),QImage::Format_ARGB32))
    {
        setBaseSize(100,100);
        ii->fill(QColor(255,255,255));
        setAutoFillBackground(false);
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
    }
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    ~VDisplay(){ delete ii; }
signals:
    void doubleClicked();
private:
    vis_t type;
    QImage *ii;
    float *bars;
    float padw, padh;
};
class VSWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit VSWidget(DockManager *manager, VPEffectPluginVis *vis, QWidget *parent = 0);
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
