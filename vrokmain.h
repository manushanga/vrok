#ifndef VROKMAIN_H
#define VROKMAIN_H

#include <QMainWindow>

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
};

#endif // VROKMAIN_H
