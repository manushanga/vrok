#ifndef TICKER_H
#define TICKER_H

#include <QWidget>

/* http://www.informit.com/articles/article.aspx?p=1405544 */

class Ticker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    Ticker(QWidget *parent = 0);

    void setText(const QString &newText);
    QString text() const { return myText; }
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private:
    bool longerThanWidth;
    QString myText;
    int offset;
    int delta;
    int myTimerId;
};


#endif // TICKER_H
