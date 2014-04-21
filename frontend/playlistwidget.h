#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include "manageddockwidget.h"
#include "dockmanager.h"
#include <QDockWidget>
#include <QDir>
#include <QFile>
#include <QStandardItemModel>
#include <QActionGroup>
#include <QTimer>

#include "vrok.h"
#include "ticker.h"

namespace Ui {
class PlaylistWidget;
}

class PlaylistWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    static void callbackNext(char *mem, void *user);
    explicit PlaylistWidget(DockManager *manager, VPlayer *vp, QWidget *parent = 0);
    void registerUi();
    QStringList getExtensionList();
    void setPlaylistTitle(QString filename);
    ~PlaylistWidget();

private slots:
    void fillQueue();
    void startFillTimer();

    void on_leSearch_textChanged(const QString &arg1);

    void on_tvLibrary_doubleClicked(const QModelIndex &index);

    void actionQueueTriggered();
    void actionSetQueueTriggered();
    void actionQueueRemove();
    void actionQueueClear();
    void actionSetLibDir();
    void actionRescan();
    void on_lvPlaylist_doubleClicked(const QModelIndex &index);

private:
    Ticker ticker;
    QString dbpath;
    QString lastplayed;
    QTimer fillTimer;
    QStandardItemModel model;
    QStandardItemModel searchModel;
    QStandardItemModel playlist;

    QList< QAction * > contextMenuFiles;
    QList< QAction * > contextMenuQueue;
    QActionGroup *queueToggleFillType;
    QActionGroup *queueTogglePlayType;
    VPlayer *player;
    Ui::PlaylistWidget *ui;
    void setTicker(QString text);
    void loadLibrary();
    void saveLibrary();
};

#endif // PLAYLISTWIDGET_H
