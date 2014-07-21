#include "playlistwidget.h"
#include "ui_playlistwidget.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QTextStream>

#include <ctime>

#define QA_REMOVE 0
#define QA_FILLRAN 1
#define QA_FILLNON 2
#define QA_FILLRPT 3
#define QA_PLAYRAN 4
#define QA_PLAYRPT 5
#define QA_PLAYSEQ 6
#define QA_CLEAR 7
#define QA_LOADPL 8
#define QA_SAVEPL 9

#define QA_QUEUE 0
#define QA_SETQUEUE 1
#define QA_SETLIBDIR 2
#define QA_RESCAN 3

void PlaylistWidget::callbackNext(char *mem, void *user)
{
    PlaylistWidget *wgt=(PlaylistWidget*)user;
    if (wgt->contextMenuQueue[QA_PLAYRAN]->isChecked()) {
        if (wgt->playlist.rowCount() > 0){
            int rowIdx=rand() % wgt->playlist.rowCount();
            QString path = wgt->playlist.item( rowIdx ,1)->text();
            wgt->lastplayed = path;
            wgt->playlist.removeRow(rowIdx);
            strcpy(mem,path.toUtf8().data());
        } else {
            QStandardItem *item = wgt->model.item(rand() % wgt->model.rowCount());
            int idx=(rand()) % item->rowCount();
            wgt->lastplayed=item->child(idx,1)->text();
            strcpy(mem, wgt->lastplayed.toUtf8().data());
        }
    } else if (wgt->contextMenuQueue[QA_PLAYRPT]->isChecked() && wgt->lastplayed != "" ) {
        strcpy(mem,wgt->lastplayed.toUtf8().data());
    } else {
        if (wgt->playlist.rowCount() > 0){
            QString path = wgt->playlist.item(0,1)->text();
            wgt->lastplayed = path;
            wgt->playlist.removeRow(0);
            strcpy(mem,path.toUtf8().data());
        } else {

            srand(time(NULL));

            if (wgt->contextMenuQueue[QA_FILLRAN]->isChecked()) {
                QStandardItem *item = wgt->model.item(rand() % wgt->model.rowCount());
                int idx=(rand()) % item->rowCount();
                wgt->lastplayed=item->child(idx,1)->text();
                strcpy(mem, wgt->lastplayed.toUtf8().data());
            } else if (wgt->contextMenuQueue[QA_FILLRPT]->isChecked() && wgt->lastplayed!="") {
                strcpy(mem,wgt->lastplayed.toUtf8().data());
            } else {
                DBG("No fill, gapless stream stopped");
            }

        }
    }
    FULL_MEMORY_BARRIER;
    wgt->metaObject()->invokeMethod(wgt,"startFillTimer");

}

PlaylistWidget::PlaylistWidget(DockManager *manager, VPlayer *vp, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    ui(new Ui::PlaylistWidget),
    player(vp)
{
    ui->setupUi(this);
    ui->tvLibrary->setColumnHidden(1,true);
    contextMenuFiles.push_back(new QAction("Queue",this));
    contextMenuFiles.push_back(new QAction("Set Queue",this));
    contextMenuFiles.push_back(new QAction("Set Library Directory",this));
    contextMenuFiles.push_back(new QAction("Rescan",this));

    ui->tvLibrary->addActions(contextMenuFiles);

    contextMenuQueue.push_back(new QAction("Remove",this));
    contextMenuQueue.push_back(new QAction("Fill Random",this));
    contextMenuQueue.push_back(new QAction("Fill None",this));
    contextMenuQueue.push_back(new QAction("Fill Repeat",this));
    contextMenuQueue.push_back(new QAction("Play Random",this));
    contextMenuQueue.push_back(new QAction("Play Repeat",this));
    contextMenuQueue.push_back(new QAction("Play Sequential",this));
    contextMenuQueue.push_back(new QAction("Clear",this));
    contextMenuQueue.push_back(new QAction("Load Playlist",this));
    contextMenuQueue.push_back(new QAction("Save Playlist",this));

    queueToggleFillType = new QActionGroup(this);
    contextMenuQueue[QA_FILLRAN]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLRAN]->setCheckable(true);
    contextMenuQueue[QA_FILLNON]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLNON]->setCheckable(true);
    contextMenuQueue[QA_FILLRPT]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLRPT]->setCheckable(true);

    queueTogglePlayType = new QActionGroup(this);
    contextMenuQueue[QA_PLAYRAN]->setActionGroup(queueTogglePlayType);
    contextMenuQueue[QA_PLAYRAN]->setCheckable(true);
    contextMenuQueue[QA_PLAYRPT]->setActionGroup(queueTogglePlayType);
    contextMenuQueue[QA_PLAYRPT]->setCheckable(true);
    contextMenuQueue[QA_PLAYSEQ]->setActionGroup(queueTogglePlayType);
    contextMenuQueue[QA_PLAYSEQ]->setCheckable(true);

    ui->lvPlaylist->addActions(contextMenuQueue);

    ui->tvLibrary->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->lvPlaylist->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(contextMenuFiles[QA_QUEUE],SIGNAL(triggered()),this,SLOT(actionQueueTriggered()));
    connect(contextMenuFiles[QA_SETQUEUE],SIGNAL(triggered()),this,SLOT(actionSetQueueTriggered()));

    connect(contextMenuFiles[QA_SETLIBDIR],SIGNAL(triggered()),this,SLOT(actionSetLibDir()));
    connect(contextMenuFiles[QA_RESCAN],SIGNAL(triggered()),this,SLOT(actionRescan()));

    connect(contextMenuQueue[QA_REMOVE],SIGNAL(triggered()),this,SLOT(actionQueueRemove()));
    connect(contextMenuQueue[QA_CLEAR],SIGNAL(triggered()),this,SLOT(actionQueueClear()));

/*    connect(contextMenuQueue[QA_LOADPL],SIGNAL(triggered()),this,SLOT(actionNewPlaylist()));
*/
    connect(&fillTimer,SIGNAL(timeout()),this,SLOT(fillQueue()));
    player->setNextTrackCallback(PlaylistWidget::callbackNext, this);
    dbpath=QString(VSettings::getSingleton()->getSettingsPath().c_str()).section('/',0,-2) ;

    int sel=VSettings::getSingleton()->readInt("playlist_menu", QA_FILLRAN);

    contextMenuQueue[sel]->setChecked(true);

    sel=VSettings::getSingleton()->readInt("playlist_menu_play", QA_PLAYSEQ);

    contextMenuQueue[sel]->setChecked(true);

    loadLibrary();

    fillTimer.setSingleShot(true);
    fillTimer.stop();
    ui->tvLibrary->setModel(&model);
    ui->lvPlaylist->setModel(&playlist);

    lastplayed="";
    ticker.setText("** Vrok **");
    setTitleBarWidget( &ticker);
    srand(time(NULL));
}

void PlaylistWidget::registerUi()
{
    dockManager->registerDockWidget(dockWidget, DockManager::TrackList);
}

QStringList PlaylistWidget::getExtensionList()
{

    std::vector<std::string> exts;
    QStringList list;
    player->getSupportedFileTypeExtensions(exts);

    for (int i=0;i<(int)exts.size();i++) {
        list.append(QString("*.").append(exts[i].c_str()));
    }
    return list;

}

void PlaylistWidget::setPlaylistTitle(QString filename)
{
}

PlaylistWidget::~PlaylistWidget()
{
    for (int i=QA_FILLRAN;i<=QA_FILLRPT;i++){
        if (contextMenuQueue[i]->isChecked()){
            VSettings::getSingleton()->writeInt("playlist_menu",i);
            break;
        }
    }

    for (int i=QA_PLAYRAN;i<=QA_PLAYSEQ;i++){
        if (contextMenuQueue[i]->isChecked()){
            VSettings::getSingleton()->writeInt("playlist_menu_play",i);
            break;
        }
    }

    delete ui;
}

void PlaylistWidget::fillQueue()
{
    int cur = playlist.rowCount();
    bool notplaying=!player->isPlaying();

    if (playlist.rowCount()<3) {
        srand(time(NULL));

        if (contextMenuQueue[QA_FILLRAN]->isChecked()) {
            for (int i=0;i<3;i++) {
                QStandardItem *item = model.item(rand() % model.rowCount());
                int idx=(rand()+i) % item->rowCount();
                playlist.setItem(cur,0, item->child(idx,0)->clone() );
                playlist.setItem(cur,1, item->child(idx,1)->clone() );
                cur++;
            }
        } else if (contextMenuQueue[QA_FILLRPT]->isChecked() && lastplayed!="") {
            playlist.setItem(cur,0,new QStandardItem(lastplayed.section('/',-1,-1)));
            playlist.setItem(cur,1,new QStandardItem(lastplayed));
            cur++;
        }
    }

    setTicker(lastplayed.section('/',-1,-1));
/*
    if (notplaying && playlist.rowCount()>0) {
        QString path = playlist.item(0,1)->text();
        playlist.removeRow(0);
        player->open(path.toUtf8().data());
    }*/
}

void PlaylistWidget::startFillTimer()
{
    fillTimer.start();
}

void PlaylistWidget::on_leSearch_textChanged(const QString &arg1)
{
    if (arg1 == ""){
        ui->tvLibrary->setModel(&model);
    } else {
        QModelIndexList list =model.match(model.index(0,0),Qt::DisplayRole,QVariant("*"+arg1+"*"),128,Qt::MatchWildcard | Qt::MatchRecursive);
        searchModel.clear();
        int i=0;
        if (list.size()) {
            QModelIndex parentIdx=list[0];
            foreach (QModelIndex idx, list) {
                QStandardItem *item=model.itemFromIndex(idx);
                if (item->parent() && parentIdx == item->parent()->index() ) {
                    continue;
                }

                if (item->hasChildren()) {
                    QStandardItem *parentItem=new QStandardItem(item->text());
                    searchModel.setItem(i,0, parentItem);
                    for (int j=0;j<item->rowCount();j++) {
                        parentItem->setChild(j,0, new QStandardItem(item->child(j,0)->text()));
                        parentItem->setChild(j,1, new QStandardItem(item->child(j,1)->text()));

                    }

                }
                else {
                    QStandardItem *pathItem=model.itemFromIndex( idx.sibling(idx.row(), 1) );

                    searchModel.setItem(i,0, new QStandardItem(item->text()));
                    searchModel.setItem(i,1, new QStandardItem(pathItem->text()));
                }
                i++;
                parentIdx=idx;
            }
        }
        ui->tvLibrary->setModel(&searchModel);
    }

}


void PlaylistWidget::on_tvLibrary_doubleClicked(const QModelIndex &index)
{
    QStandardItemModel *model=(QStandardItemModel *)ui->tvLibrary->model();
    QStandardItem *item=model->itemFromIndex(index);
    if (!item->hasChildren()) {
        lastplayed=index.sibling(item->row(),1).data().toString();  
        player->open( VPResource( std::string(lastplayed.toUtf8().data()), VPResource::INIT_FILE ) ,false);
        setTicker(lastplayed.section('/',-1,-1));
    }
}

void PlaylistWidget::actionQueueTriggered()
{

    QModelIndexList idxlist=ui->tvLibrary->selectionModel()->selectedIndexes();
    QStandardItemModel *model = (QStandardItemModel *)ui->tvLibrary->model();
    int cur=playlist.rowCount();
    foreach(QModelIndex idx, idxlist) {
        QStandardItem *item=model->itemFromIndex(idx);
        if (item->column() == 0) {
            if (item->hasChildren()) {
                for (int i=0;i<item->rowCount();i++) {
                    playlist.setItem(cur,0, item->child(i,0)->clone());
                    playlist.setItem(cur,1,item->child(i,1)->clone());
                    cur++;
                }
            } else {
                playlist.setItem(cur,0, item->clone());
                playlist.setItem(cur,1, model->itemFromIndex(idx.sibling(item->row(),1))->clone());
                cur++;
            }
        }

    }
}

void PlaylistWidget::actionSetQueueTriggered()
{
    playlist.clear();
    actionQueueTriggered();
}

void PlaylistWidget::actionQueueRemove()
{
    ui->lvPlaylist->setUpdatesEnabled(false);
    QModelIndexList list=ui->lvPlaylist->selectionModel()->selectedIndexes();
    qSort(list);
    for (int i=list.count()-1;i>=0;i--){
        playlist.removeRow(list[i].row());
    }
    ui->lvPlaylist->setUpdatesEnabled(true);
}

void PlaylistWidget::actionQueueClear()
{
    playlist.clear();
}

void PlaylistWidget::actionSetLibDir()
{

    QString d = QFileDialog::getExistingDirectory(this, tr("Open Dir"),
                                                  QString(VSettings::getSingleton()->readString("lastopen","").c_str()),
                                                  0);
    VSettings::getSingleton()->writeString("lastopen",d.toStdString());
    actionRescan();
}

void PlaylistWidget::actionRescan()
{
    model.clear();
    QFile file(dbpath+"/ml.db");
    file.remove();
    loadLibrary();
}

void PlaylistWidget::on_lvPlaylist_doubleClicked(const QModelIndex &index)
{
    QStandardItemModel *model=(QStandardItemModel *)ui->lvPlaylist->model();
    QStandardItem *item=model->itemFromIndex(index);
    if (!item->hasChildren()) {
        lastplayed=index.sibling(item->row(),1).data().toString();
        player->open( VPResource( std::string(lastplayed.toUtf8().data()), VPResource::INIT_FILE ),false);
        if (contextMenuQueue[QA_FILLRPT]->isChecked()) {
            int r=model->rowCount();
            model->setItem(r,0, item->clone());
            model->setItem(r,1, model->itemFromIndex(index.sibling(item->row(),1))->clone() );
        }
        setTicker(lastplayed.section('/',-1,-1));
        model->removeRow(index.row());
    }
}

void PlaylistWidget::setTicker(QString text)
{
    ticker.setText(text);
}

void PlaylistWidget::loadLibrary()
{

    QFile file(dbpath+"/ml.db");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QTextStream ts(&file);
        ts.setCodec("utf-8");
        int dirs=0,tracks=0;
        dirs = ts.readLine().toInt();
        DBG(dirs);
        for (int i=0;i<dirs;i++) {
            QString dir=ts.readLine();
            QStandardItem *item = new QStandardItem(dir);
            model.setItem(model.rowCount(),item);

            tracks = ts.readLine().toInt();
            DBG(tracks);
            for (int j=0;j<tracks;j++){
                QString track=ts.readLine();
                item->setChild(j,0,new QStandardItem(track.section('/',-1,-1)));
                item->setChild(j,1,new QStandardItem(track));
            }

        }
        file.close();
    } else {
        std::string lastpath=VSettings::getSingleton()->readString("lastopen","");

        if (lastpath.size() !=0){
            model.clear();
            QString currentPath(lastpath.c_str());
            QString rootPath=currentPath;
            QStringList extensions=getExtensionList();
            QDirIterator iterator(currentPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );
            QDirIterator iteratorSubRoot(currentPath,extensions, QDir::NoDotAndDotDot | QDir::Files);

            if (iteratorSubRoot.hasNext()){

                QStandardItem *dirItem=new QStandardItem(".");
                model.setItem(model.rowCount(),dirItem);

                int i=0;

                while (iteratorSubRoot.hasNext()){
                    iteratorSubRoot.next();
                    dirItem->setChild(i,0, new QStandardItem(iteratorSubRoot.fileName()));
                    dirItem->setChild(i,1, new QStandardItem(iteratorSubRoot.filePath()));
                    i++;
                }
            }

            while (iterator.hasNext()) {
               iterator.next();
               if (iterator.fileInfo().isDir()) {
                   QDirIterator iteratorSub(iterator.filePath(),extensions, QDir::NoDotAndDotDot | QDir::Files);
                   if (iteratorSub.hasNext()){
                       QStandardItem *dirItem=new QStandardItem(iterator.filePath().right(iterator.filePath().length() - rootPath.length() - 1));
                       model.setItem(model.rowCount(),dirItem);

                       QDirIterator iteratorx(iterator.filePath(),extensions,QDir::NoDotAndDotDot | QDir::Files);

                       int i=0;

                       while (iteratorx.hasNext()){
                           iteratorx.next();
                           dirItem->setChild(i,0, new QStandardItem(iteratorx.fileName()));
                           dirItem->setChild(i,1, new QStandardItem(iteratorx.filePath()));
                           i++;
                       }
                   }
               }
            }
        }

        saveLibrary();
    }
}

void PlaylistWidget::saveLibrary()
{
    QFile file(dbpath+"/ml.db");
    file.open(QFile::WriteOnly);
    QTextStream ts(&file);
    ts.setCodec("utf-8");
    ts << model.rowCount() << "\n";
    for (int i=0;i<model.rowCount();i++) {
        QStandardItem *item = model.item(i);
        ts<< item->text() <<"\n";
        if (item->hasChildren()) {
            ts << item->rowCount() << "\n";
            for (int j=0;j<item->rowCount();j++){
                QStandardItem *childItem = item->child(j,1);
                ts << childItem->text() <<"\n";
            }
        }
    }
    file.close();
}
