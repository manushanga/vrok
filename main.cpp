#include <unistd.h>

#include <QApplication>
#include "vrokmain.h"
#include "vplayer.h"
#include <vector>

int main(int argc, char *argv[])
{
    std::vector<int> ax;
    ax.push_back(1);
    QApplication a(argc, argv);
    VrokMain w;
    w.show();
    return a.exec();
}
