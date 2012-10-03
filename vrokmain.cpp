#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vplayer.h"

VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain)
{
    ui->setupUi(this);

}

VrokMain::~VrokMain()
{
    delete ui;
}
