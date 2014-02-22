#include "playlistwidget.h"
#include "ui_playlistwidget.h"

#include <QDirIterator>

PlaylistWidget::PlaylistWidget(DockManager *manager, VPlayer *vp, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    ui(new Ui::PlaylistWidget),
    player(vp)
{
    ui->setupUi(this);
    ui->tvLibrary->setColumnHidden(1,true);

    QString dbfile=QString(VSettings::getSingleton()->getSettingsPath().c_str()).section('/',0,-2) ;
    dbfile += "/ml.db";
  /*  if (QFile(dbfile).exists()) {
    } else {
*/
        std::string lastpath=VSettings::getSingleton()->readString("lastopen","");
        if (lastpath.size() !=0){
            QString currentPath(lastpath.c_str());
            QString rootPath=currentPath;
            QStringList extensions=getExtensionList();
            QDirIterator iterator(currentPath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories );
            QDirIterator iteratorSubRoot(currentPath,extensions, QDir::NoDotAndDotDot | QDir::Files);

            if (iteratorSubRoot.hasNext()){
                QStandardItem *dirItem=new QStandardItem(currentPath.right(currentPath.length() - rootPath.length()));
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

            while (iterator.hasNext()) {
               iterator.next();
               if (iterator.fileInfo().isDir()) {
                   QDirIterator iteratorSub(iterator.filePath(),extensions, QDir::NoDotAndDotDot | QDir::Files);
                   if (iteratorSub.hasNext()){
                       QStandardItem *dirItem=new QStandardItem(iterator.filePath().right(iterator.filePath().length() - rootPath.length()));
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
    //}
    ui->tvLibrary->setModel(&model);
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

PlaylistWidget::~PlaylistWidget()
{
    delete ui;
}

int PlaylistWidget::db_callback(void *user, int argc, char **argv, char **azColName)
{
    return 0;
}

void PlaylistWidget::on_leSearch_textChanged(const QString &arg1)
{
    if (arg1 == ""){
        ui->tvLibrary->setModel(&model);
    } else {
        QModelIndexList list =model.match(model.index(0,0),Qt::DisplayRole,QVariant(arg1),128,Qt::MatchWildcard | Qt::MatchRecursive);
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

void PlaylistWidget::on_lvPlaylist_clicked(const QModelIndex &index)
{


}

void PlaylistWidget::on_tvLibrary_doubleClicked(const QModelIndex &index)
{
    QStandardItemModel *model=(QStandardItemModel *)ui->tvLibrary->model();
    QStandardItem *item=model->itemFromIndex(index);
    if (!item->hasChildren()) {
        player->open(index.sibling(item->row(),1).data().toString().toStdString().c_str(),false);
    }
}

void PlaylistWidget::on_tvLibrary_clicked(const QModelIndex &index)
{

}
