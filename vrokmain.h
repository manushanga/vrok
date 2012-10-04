#ifndef VROKMAIN_H
#define VROKMAIN_H

#include <QMainWindow>

#include <vplayer.h>

namespace Ui {
class VrokMain;
}

class VrokMain : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit VrokMain(QWidget *parent = 0);
    ~VrokMain();
    
private:
    Ui::VrokMain *ui;
    VPlayer *vp;
};

#endif // VROKMAIN_H
