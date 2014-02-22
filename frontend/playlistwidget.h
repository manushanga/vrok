#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include "manageddockwidget.h"
#include "dockmanager.h"
#include <QDockWidget>
#include <QDir>
#include <QFile>
#include <QStandardItemModel>

#include "vrok.h"
#include "sqlite3.h"

namespace Ui {
class PlaylistWidget;
}

class PlaylistWidget : public ManagedDockWidget
{
    Q_OBJECT

public:
    explicit PlaylistWidget(DockManager *manager, VPlayer *vp, QWidget *parent = 0);
    void registerUi();
    QStringList getExtensionList();
    ~PlaylistWidget();

private slots:
    void on_leSearch_textChanged(const QString &arg1);

    void on_lvPlaylist_clicked(const QModelIndex &index);

    void on_tvLibrary_doubleClicked(const QModelIndex &index);

    void on_tvLibrary_clicked(const QModelIndex &index);

private:
    static int db_callback(void *user, int argc, char **argv, char **azColName);
    QStandardItemModel model;
    QStandardItemModel searchModel;
    QStandardItemModel playlist;
    sqlite3 *db;
    VPlayer *player;
    Ui::PlaylistWidget *ui;
};

#endif // PLAYLISTWIDGET_H
