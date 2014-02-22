#ifndef DISPLAYTICKER_H
#define DISPLAYTICKER_H
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QList>
#include <QLabel>
#include <QPainter>
#include <QKeyEvent>
#include <QWidget>

#include "vrok.h"

class DisplayTicker : public QWidget
{
    Q_OBJECT
private:
    QTimer t;
    int x;
    QString _text;
public:
    DisplayTicker(QWidget *parent=0) : QWidget(parent) {
        setFixedHeight(fontMetrics().height());
        t.setSingleShot(false);
        t.setInterval(500);
        connect(&t, SIGNAL(timeout()), this,SLOT(updateScrollingText()));
        x=0;
        t.stop();
    }
    inline void setText(QString text){
        _text = text;
    }
    inline QString& text(){
        return _text;
    }
public slots:
    void updateScrollingText(){
        x-=10;
        if (x<0) {
            x=width();
        }
        repaint();
    }
protected:
    void resizeEvent(QResizeEvent *){
        x=width();
    }
    void paintEvent(QPaintEvent *evt) {
        QPainter painter(this);

        int textWidth =fontMetrics().width(_text) ;
        if (textWidth < width() && x + textWidth> width()) {
            painter.drawText(x-width(),fontMetrics().height(),_text);
        };
        painter.drawText(x,fontMetrics().height(),_text);


    }

};

#endif // DISPLAYTICKER_H
