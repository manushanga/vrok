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
        t.start();
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
        QImage buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);

        buffer.fill(qRgba(0, 0 ,0, 0));
        QPainter pb(&buffer);
        QPen s(Qt::black);
        pb.setPen(painter.pen());
        pb.setFont(painter.font());
        int textWidth =fontMetrics().width(_text) ;
        if (textWidth < width() && x + textWidth> width()) {
            pb.drawText(x-width(),fontMetrics().height(),_text);
        };
        pb.drawText(x,fontMetrics().height(),_text);

        painter.drawImage(0,0,buffer);

    }

};

#endif // DISPLAYTICKER_H
