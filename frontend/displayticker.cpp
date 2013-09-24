#include "displayticker.h"

DisplayTicker::DisplayTicker(QObject *parent) :
    QObject(parent)

{
    lbl = NULL;
    song = "";
    info = "";
    start = "  ";
    end = "  e  ";
    textOut = "  ";

    t.setInterval(500);
    t.setSingleShot(false);
    QObject::connect(&t, SIGNAL(timeout()), this, SLOT(update()));
    t.start();
}

DisplayTicker::~DisplayTicker()
{
    t.stop();
}

void DisplayTicker::update()
{
    if (!lbl)
        return;
    if (textOut.length())
        textOut = textOut.right(textOut.length() -1).append(textOut.at(0));

    lbl->setText(textOut);

}
