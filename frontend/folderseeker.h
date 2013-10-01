#ifndef FOLDERSEEKER_H
#define FOLDERSEEKER_H

#include <QDir>
#include <QStandardItemModel>
#include <QString>

class FolderSeeker
{
public:
    static FolderSeeker* getSingleton();
    FolderSeeker();
    void setExtensionList(QStringList _extensions);
    void setSeekPath(QString path);
    QString getQueue(QStandardItemModel *m, int idx);
    inline int getFolderCount(){
        return dirs.size();
    }
    inline QString getSeekPath() {
        return currentPath;
    }
    ~FolderSeeker();

private:
    void folderSeekSweep();
    QStringList extensions;
    QString currentPath;
    QStringList dirs;
};

#endif // FOLDERSEEKER_H
