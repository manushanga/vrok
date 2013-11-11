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

    VDisplay(VPEffectPluginVis::vis_t vis_type, VPEffectPluginVis *eff, QWidget *parent=0) :
        QWidget(parent),
        plugin(eff),
        type(vis_type),
        ii(new QImage(width(),height(),QImage::Format_ARGB32))
    {
        setBaseSize(100,100);
        ii->fill(QColor(255,255,255));
        setAutoFillBackground(false);
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
    }
    void setType(VPEffectPluginVis::vis_t  vis_type){ type = vis_type; }
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    ~VDisplay(){ delete ii; }
signals:
    void doubleClicked();
private:
    VPEffectPluginVis::vis_t  type;
    QImage *ii;
    VPEffectPluginVis *plugin;
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
    VPEffectPluginVis::vis_t type;
    VDisplay disp;
    VPEffectPluginVis *plugin;
    QTimer q;
    Ui::VSWidget *ui;
};

#endif // VSWIDGET_H
