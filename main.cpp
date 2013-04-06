/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#include <QApplication>
#include <QTranslator>
#include "frontend/vrokmain.h"
#include "vplayer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;

    translator.load("./vrok_si");
    a.installTranslator(&translator);

    VrokMain w;
    w.show();
    return a.exec();
}
