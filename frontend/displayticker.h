#ifndef DISPLAYTICKER_H
#define DISPLAYTICKER_H
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QList>
#include <QLabel>

#include "vrok.h"

class DisplayTicker : public QObject
{
    Q_OBJECT
private:
    QList<QChar> scrollingTicker;
    QTimer t;
    QString start, end, ticker, song, info, textOut;
    QLabel *lbl;
public:
    static const double ratio=0.005;
    DisplayTicker(QObject *parent =0);
    inline void setLabel(QLabel *label){
        lbl = label;
    }
    inline void setSongName(QString name) {
        t.stop();
        song = name;
        textOut = QString( start + song + info + end).repeated(1+lbl->width() * ratio) ;
        t.start();
    }
    inline void setInfo(QString text) {
        t.stop();
        info = text;
        textOut = QString( start + song + info +  end).repeated(1+lbl->width() * ratio) ;
        t.start();
    }
    inline void resize(int size){
        t.stop();
        textOut = QString( start + song + info + end).repeated(1+size * ratio) ;
        t.start();
    }
    inline QString& getText(){
        return textOut;
    }
    ~DisplayTicker();
public slots:
    void update();
};

#endif // DISPLAYTICKER_H
