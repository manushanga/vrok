#include <unistd.h>

#include <QApplication>
#include "vrokmain.h"
#include "vplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VrokMain w;
    w.show();
    return a.exec();
}
