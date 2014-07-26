#include "controlswidget.h"
#include "ui_controlswidget.h"

void ControlsWidget::OnStateChangePlaying(void *message, int messageLength, void *user)
{

    ControlsWidget *wgt = (ControlsWidget *)user;
    wgt->ui->btPlay->setIcon(*wgt->pause_icon);
}

void ControlsWidget::OnStateChangePaused(void *message, int messageLength, void *user)
{
    ControlsWidget *wgt = (ControlsWidget *)user;
    wgt->ui->btPlay->setIcon(*wgt->play_icon);
}

ControlsWidget::ControlsWidget(DockManager *manager, VrokMain *vrokMain, QWidget *parent) :
    ManagedDockWidget(manager, this, parent),
    main(vrokMain),
    ui(new Ui::ControlsWidget)
{
    connect(&tpos,SIGNAL(timeout()),this,SLOT(positionTick()));

    tpos.setSingleShot(false);
    tpos.setInterval(1000);
    tpos.start();
    play_icon=new QIcon(":icon/res/play");
    pause_icon=new QIcon(":icon/res/pause");
    VPEvents::getSingleton()->addListener("StateChangePlaying",(VPEvents::VPListener)OnStateChangePlaying,this);
    VPEvents::getSingleton()->addListener("StateChangePaused",(VPEvents::VPListener)OnStateChangePaused,this);

    ui->setupUi(this);
}

void ControlsWidget::registerUi()
{
    dockManager->registerDockWidget(dockWidget, DockManager::ControlBay);
}

ControlsWidget::~ControlsWidget()
{
    delete ui;
}

void ControlsWidget::on_sbPosition_sliderReleased()
{
    if (main->vp->isPlaying())
        main->vp->setPosition(ui->sbPosition->value()/1000.0f);
}

void ControlsWidget::on_btPlay_clicked()
{
    if (main->vp->isPlaying()){
        main->vp->pause();
    } else {
        main->vp->play();
    }
}

void ControlsWidget::on_btAbout_clicked()
{
    main->showAboutVrok();
}

void ControlsWidget::positionTick()
{
    if (main->vp->isPlaying())
        ui->sbPosition->setValue(main->vp->getPosition()*1000);
}

void ControlsWidget::on_btPlugins_clicked()
{
    main->configurePlugins();
}
