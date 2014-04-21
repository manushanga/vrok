#include <QtGui>

#include "ticker.h"

Ticker::Ticker(QWidget *parent)
    : QWidget(parent)
{
    offset = 0;
    myTimerId = 0;
    delta = 0;
}

void Ticker::setText(const QString &newText)
{
    myText = newText;
    update();
    updateGeometry();
    if (fontMetrics().width(myText) +10 > width())
        longerThanWidth=true;
    else
        longerThanWidth=false;
}

QSize Ticker::sizeHint() const
{
    QSize sz=fontMetrics().size(0, text());
    sz.setHeight(sz.height() + 6);
    return sz;
}

void Ticker::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    int textWidth = fontMetrics().width(text())+10;
    if (textWidth < 1)
        return;
    painter.drawText(offset, 3, textWidth, height(),
                         Qt::AlignLeft | Qt::AlignVCenter, text());
}

void Ticker::showEvent(QShowEvent * /* event */)
{
    myTimerId = startTimer(300);
}

void Ticker::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == myTimerId) {
        offset+=delta;
        if (longerThanWidth) {
            if (offset + fontMetrics().width(text()) < width())
                delta=10;
            if (offset >=0 )
                delta=-10;

        } else {
            if (offset + fontMetrics().width(text()) >= width())
                delta=-10;
            if (offset <= 0)
                delta=10;
        }
        update();
    } else {
        QWidget::timerEvent(event);
    }
}

void Ticker::hideEvent(QHideEvent * /* event */)
{
    killTimer(myTimerId);
    myTimerId = 0;
}
