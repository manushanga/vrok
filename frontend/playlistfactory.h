#ifndef PLAYLISTFACTORY_H
#define PLAYLISTFACTORY_H

#include "vrok.h"
#include <QStringListModel>
#include <QStandardItemModel>
#include <QList>
#include <QDir>
#include <QFile>
#include <QTextStream>
class PlaylistFactory {
private:
    QString root;
public:
    static PlaylistFactory *getSingleton()
    {
        static PlaylistFactory pf;
        return &pf;
    }
    void setRoot(QString rootDir){ root=rootDir; }
    PlaylistFactory();
    int loadQueue(QStandardItemModel *m, int idx);
    int saveQueue(QStandardItemModel *m, int idx);


};
#endif // PLAYLISTFACTORY_H
