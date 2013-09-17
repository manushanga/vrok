
#include "playlistfactory.h"

PlaylistFactory::PlaylistFactory()
{

}

int PlaylistFactory::loadQueue(QStandardItemModel *m, int idx)
{
    m->clear();
    QFile f(root + QDir::separator() + ".vrok." +QString::number(idx) + ".queue");

    f.open(QFile::Text | QFile::ReadOnly);
    if (f.exists()) {
        f.setTextModeEnabled(true);

        QTextStream ts(&f);
        int i=0;
        ts.setCodec("UTF-8");
        while (!ts.atEnd()) {
            QString path = ts.readLine();
            m->setItem(i,0,new QStandardItem(path.section(QDir::separator(),-1,-1)));
            m->setItem(i,1,new QStandardItem(path));
            i++;
        }
        f.close();
        return 1;
    } else {
        return 0;
    }

}

int PlaylistFactory::saveQueue(QStandardItemModel *m, int idx)
{

    QFile f(root + QDir::separator() + ".vrok." +QString::number(idx) + ".queue");
    f.open(QFile::Text | QFile::WriteOnly);
    f.setTextModeEnabled(true);
    QTextStream ts(&f);
    QString end("\n");
    int i=0;
    ts.setCodec("UTF-8");
    while (m->rowCount() > i) {
        ts << m->item(i,1)->text() + end ;
        i++;
    }
    f.close();
    return 1;


}

