/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#include <QDirIterator>
#include <QDialog>
#include <QLinearGradient>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QMessageBox>

#ifdef __linux__
#include <unistd.h>
#endif

#include "vrokmain.h"
#include "ui_vrokmain.h"
#include "vputils.h"
#include "vplayer.h"
#include "vswidget.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"
#include "effects/shibatch/eq.h"

#include "playlistfactory.h"

#include <cstring>
#include <ctime>

#define QA_REMOVE 0
#define QA_FILLRAN 1
#define QA_FILLSEQ 2
#define QA_FILLNON 3
#define QA_FILLLIMITDIR 4
#define QA_NEW_QUEUE 5

#define QA_QUEUE 0

void VrokMain::callback_next(char *mem, void *user)
{
    VrokMain *mm =(VrokMain *) user;

    if (mm->queueModel.rowCount() > 0){
        QString name = mm->queueModel.item(0,0)->text();
        QString path = mm->queueModel.item(0,1)->text();
        mm->queueModel.removeRow(0);
        mm->lblDisplay->setText(name);
        strcpy(mem,path.toUtf8().data());
    }

    if (mm->queueModel.rowCount() < 3) {
        mm->metaObject()->invokeMethod(mm,"startFillTimer");
    }
}

bool VrokMain::eventFilter(QObject *target, QEvent *event)
{
    if ( event->type() == QEvent::MouseButtonPress ){
        return true;
    }
    if ((target == (QObject *)ui->lvFiles) && (event->type() == QEvent::KeyPress)) {
        int val = ui->sbFolderSeek->value();
        switch (((QKeyEvent*)event)->key()) {
        case Qt::Key_Left:
            if (val > 0)
                ui->sbFolderSeek->setValue(val-1);
            return true;
        case Qt::Key_Right:
            if (val < ui->sbFolderSeek->maximum())
                ui->sbFolderSeek->setValue(val+1);
            return true;
        case Qt::Key_Enter:
            on_lvFiles_doubleClicked(ui->lvFiles->currentIndex());
            return true;
        case Qt::Key_Return:
            on_lvFiles_doubleClicked(ui->lvFiles->currentIndex());
            return true;
        default:
            return false;
        }

    } else {
        return false;
    }
}

void VrokMain::resizeEvent(QResizeEvent *event)
{

}

void VrokMain::on_btnAbout_clicked()
{
    QDialog d(this);
    d.setWindowTitle(tr("About Vrok"));
    d.setModal(true);
    QHBoxLayout h(&d);
    d.setLayout(&h);
    QLabel a("<center>"
             "<span style=\"font-size: 12pt\"><b>Vrok</b> smokin' audio<br/></span>"
             "</center>"
             "Copyright (C) 2012-2013 Madura A. <madura.x86@gmail.com><br/>"
             "<br/>"
             "<b>Libraries</b><br/>"
             "<br/>Sound Ouput<br/>"
             "<a href=\"http://www.alsa-project.org\">libasound</a>: Advanced Linux Sound Architecture<br/>"
             "<a href=\"http://mega-nerd.com/SRC/\">libsamplerate</a>: Samplerate convertor(ALSA only)<br/>"
             "<a href=\"http://pulseaudio.org\">libpulse-simple</a>: PulseAudio Synchronous API<br/>"
             "<a href=\"http://msdn.microsoft.com/en-us/library/windows/desktop/ee416960%28v=vs.85%29.aspx\">DSound</a>: Windows sound output<br/>"
             "<br/>Decoders<br/>"
             "<a href=\"http://flac.sourceforge.net/\">libFLAC</a>: FLAC Decoder<br/>"
             "<a href=\"http://xiph.org/vorbis/\">libvorbisfile</a>: OGG Vorbis Decoder<br/>"
             "<a href=\"http://www.mpg123.de/\">libmpg123</a>: MPEG Layer 1,2,3 Decoder<br/>"
             "<br/>DSP<br/>"
             "<a href=\"http://shibatch.sourceforge.net/\">SuperEQ</a>: Naoki Shibata's 18 Band Equalizer<br/>"
             "<br/>GUI<br/>"
             "<a href=\"http://qt-project.org\">Qt</a>: Frontend<br/>"
             "<br/>"
             "<span style=\"font-size: 8pt\">Bugs may exist. Built on " __DATE__ " at " __TIME__ ".</span>");
    h.addWidget(&a);
    d.exec();
}
VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain),
    dockManager(this),
    lastTab(0)
{
    ui->setupUi(this);

    lblDisplay = new DisplayTicker(this);
    ui->vlColumn2->addWidget(lblDisplay);

    vp=NULL;
    ew=NULL;
    vw=NULL;

    setWindowIcon(QIcon(":icon/vrok.png"));

    vp = new VPlayer(callback_next, this);
    eq = new VPEffectPluginEQ(100);
    /*
    VSWidget *vs = new VSWidget();
    setDockOptions(QMainWindow::ForceTabbedDocks);
    VSWidget *u=new VSWidget();
    VSWidget *xx=new VSWidget();

    addDockWidget(Qt::BottomDockWidgetArea,vs);
    addDockWidget(Qt::BottomDockWidgetArea,u);
    addDockWidget(Qt::BottomDockWidgetArea,xx);

    tabifyDockWidget(vs, u);
    tabifyDockWidget(vs,xx);
*/
    if (VSettings::getSingleton()->readInt("eqon",1) ){
        vp->addEffect((VPEffectPlugin *)eq);
        ui->btnEQt->setChecked(true);
    }

    tx.setSingleShot(false);
    tx.setInterval(40);
    tx.stop();

    tcb.setSingleShot(true);
    tcb.setInterval(0);
    tcb.stop();

    tpos.setSingleShot(false);
    tpos.setInterval(1000);
    tpos.start();

    curdir.setFilter(QDir::Files|QDir::Hidden);
    curdir.setNameFilters(getExtentionsList());
    cursweep.setFilter(QDir::Files);
    cursweep.setNameFilters(getExtentionsList());

    gs = new QGraphicsScene();
    QLinearGradient gr(0,-100,0,100);
    gr.setColorAt(0,QColor(255,0,0));
    gr.setColorAt(0.15,QColor(200,190,5));
    gr.setColorAt(0.5,QColor(0,150,0));
    gr.setInterpolationMode(QGradient::ColorInterpolation);

    QLinearGradient gr2(0,-100,0,100);
    gr2.setColorAt(0,QColor(100,0,0));
    gr2.setColorAt(0.15,QColor(50,95,2));
    gr2.setColorAt(0.5,QColor(0,75,0));
    gr2.setInterpolationMode(QGradient::ColorInterpolation);

    QBrush z(gr);
    QBrush y(gr2);
    QPen x(Qt::transparent);

    for (unsigned i=0;i<BAR_COUNT;i++){
        gbars[i] = new QGraphicsRectItem(i*11,0,10,0);
        gbars[i]->setBrush(z);
        gbars[i]->setPen(x);
        gs->addItem(gbars[i]);

        gmbars[i] = new QGraphicsRectItem(i*12,1,2,0);
        gmbars[i]->setBrush(y);
        gmbars[i]->setPen(x);
        gs->addItem(gmbars[i]);

        gbbars[i] = new QGraphicsRectItem(i*12,1,2,0);
        gbbars[i]->setBrush(QBrush(QColor(255,0,50)));
        gbbars[i]->setPen(x);

        gs->addItem(gbbars[i]);

        bar_vals[i]=0.0f;
    }
    //ui->gvDisplay->setScene(gs);

    FolderSeeker::getSingleton()->setExtensionList(getExtentionsList());
    if (( VSettings::getSingleton()->readString("lastopen","") ).size()>0) {
        ui->lvFiles->setModel(&dirFilesModel);

        lblDisplay->setText("Scanning...");
        QApplication::instance()->processEvents();
        FolderSeeker::getSingleton()->setSeekPath(QString(VSettings::getSingleton()->readString("lastopen","").c_str()));
        lblDisplay->setText("Done.");
        QApplication::instance()->processEvents();
        ui->sbFolderSeek->setMinimum(0);
        ui->sbFolderSeek->setMaximum(FolderSeeker::getSingleton()->getFolderCount() - 1);

        on_sbFolderSeek_valueChanged(0);
        PlaylistFactory::getSingleton()->setRoot(FolderSeeker::getSingleton()->getSeekPath());
    }
    ui->lvFiles->installEventFilter(this);
    vis_counter = 0;
    vp->effectsActive = true;

    contextMenuFiles.push_back(new QAction("Queue",this));

    ui->lvFiles->addActions(contextMenuFiles);

    contextMenuQueue.push_back(new QAction("Remove",this));
    contextMenuQueue.push_back(new QAction("Fill Random",this));
    contextMenuQueue.push_back(new QAction("Fill Sequential",this));
    contextMenuQueue.push_back(new QAction("Fill None",this));
    contextMenuQueue.push_back(new QAction("Fill from directory",this));
    contextMenuQueue.push_back(new QAction("New Queue",this));

    queueToggleFillType = new QActionGroup(this);
    contextMenuQueue[QA_FILLRAN]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLRAN]->setCheckable(true);
    contextMenuQueue[QA_FILLSEQ]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLSEQ]->setCheckable(true);
    contextMenuQueue[QA_FILLNON]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLNON]->setCheckable(true);
    contextMenuQueue[QA_FILLLIMITDIR]->setActionGroup(queueToggleFillType);
    contextMenuQueue[QA_FILLLIMITDIR]->setCheckable(true);
    ui->lvQueue->addActions(contextMenuQueue);

    ui->lvFiles->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->lvQueue->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->lvQueue->setModel(&queueModel);
    ui->lvFiles->setModel(&dirFilesModel);

    connect(contextMenuFiles[QA_QUEUE],SIGNAL(triggered()),this,SLOT(actionQueueTriggered()));
    connect(contextMenuQueue[QA_REMOVE],SIGNAL(triggered()),this,SLOT(actionQueueRemove()));
    connect(contextMenuQueue[QA_NEW_QUEUE],SIGNAL(triggered()),this,SLOT(actionNewQueue()));

    connect(&tx, SIGNAL(timeout()), this, SLOT(process()));
    connect(&tcb,SIGNAL(timeout()), this, SLOT(fillQueue()));
    connect(&tpos,SIGNAL(timeout()),this,SLOT(positionTick()));

    lblDisplay->setText(" *smoke* ... V r o k ... *some more smoke* ");
    // temp stuff until settings are written
    contextMenuQueue[QA_FILLRAN]->setChecked(true);

}
void VrokMain::startFillTimer()
{
    tcb.start();
}

void VrokMain::positionTick()
{
    if (vp->isPlaying())
        ui->sbPosition->setValue(vp->getPosition()*1000);
}
void VrokMain::fillQueue()
{
    srand(time(NULL));
    if (contextMenuQueue[QA_FILLRAN]->isChecked()) {
        QStandardItemModel fileModel;
        int j=0;
        uint dbloom=0,drbloom;
        if (FolderSeeker::getSingleton()->getFolderCount()==1) {
            FolderSeeker::getSingleton()->getQueue(&fileModel,0);
            for (int i=0;i<fileModel.rowCount();i++) {
                int rc = queueModel.rowCount();
                queueModel.setItem(rc,0, fileModel.item(i,0)->clone());
                queueModel.setItem(rc,1, fileModel.item(i,1)->clone());
            }
        } else {
            while (queueModel.rowCount()<10 && j<100){
                drbloom = abs(rand()) % (FolderSeeker::getSingleton()->getFolderCount()-1);
                if ((drbloom & dbloom) != drbloom) {
                    FolderSeeker::getSingleton()->getQueue(&fileModel,drbloom);
                    uint bloom=0;

                    int selectMax=(fileModel.rowCount()>4)?4:2;
                    for (int i=0;i<selectMax;i++) {
                        int rc = queueModel.rowCount();
                        int tr = abs(rand()*5+rand()+i) % (fileModel.rowCount());
                        uint rbloom = FNV(fileModel.item(tr,1)->text().toUtf8().data());
                        if ((rbloom & bloom) != rbloom) {

                            queueModel.setItem(rc,0, fileModel.item(tr,0)->clone());
                            queueModel.setItem(rc,1, fileModel.item(tr,1)->clone());

                            bloom = bloom | rbloom;
                        }
                    }
                    dbloom = dbloom | drbloom;
                }
                j++;
            }
        }
        if (queueModel.rowCount()>0 && !vp->isPlaying()) {
            QString path = queueModel.item(0,1)->text();
            lblDisplay->setText(queueModel.item(0,0)->text());
            queueModel.removeRow(0);
            vp->open(path.toUtf8().data());
        }
    } else if (contextMenuQueue[QA_FILLSEQ]->isChecked()) {
        QStandardItemModel fileModel;
        uint dbloom=0,drbloom;
        if (FolderSeeker::getSingleton()->getFolderCount()==1) {
            PlaylistFactory::getSingleton()->loadQueue(&fileModel,0);
            for (int i=0;i<fileModel.rowCount();i++) {
                int rc = queueModel.rowCount();
                queueModel.setItem(rc,0, fileModel.item(i,0)->clone());
                queueModel.setItem(rc,1, fileModel.item(i,1)->clone());
            }
        } else {
            while (queueModel.rowCount()<10){
                drbloom = abs(rand()) % (FolderSeeker::getSingleton()->getFolderCount()-1);
                if ((drbloom & dbloom) != drbloom) {
                    PlaylistFactory::getSingleton()->loadQueue(&fileModel,drbloom);

                    for (int i=0;i<fileModel.rowCount();i++) {
                        int rc = queueModel.rowCount();
                        queueModel.setItem(rc,0, fileModel.item(i,0)->clone());
                        queueModel.setItem(rc,1, fileModel.item(i,1)->clone());

                    }
                    dbloom = dbloom | drbloom;
                }
            }
        }
        if (queueModel.rowCount()>0 && !vp->isPlaying()) {
            QString path = queueModel.item(0,1)->text();
            lblDisplay->setText(queueModel.item(0,0)->text());
            queueModel.removeRow(0);
            vp->open(path.toUtf8().data());
        }
    } else if (contextMenuQueue[QA_FILLLIMITDIR]->isChecked()) {
        QStandardItemModel fileModel;
        int j=0;
        uint dbloom=0,drbloom;
        FolderSeeker::getSingleton()->getQueue(&fileModel, ui->sbFolderSeek->value());

        QSet<int> checker;
        int selectMax=(fileModel.rowCount() < 100)? fileModel.rowCount():100;
        for (int i=0;i<selectMax;i++) {
            int rc = queueModel.rowCount();
            int tr = abs(rand()*5+rand()+i) % (fileModel.rowCount());

            if (checker.find(tr) == checker.end()) {

                queueModel.setItem(rc,0, fileModel.item(tr,0)->clone());
                queueModel.setItem(rc,1, fileModel.item(tr,1)->clone());

                checker.insert(tr);
            }
        }


        if (queueModel.rowCount()>0 && !vp->isPlaying()) {
            QString path = queueModel.item(0,1)->text();
            lblDisplay->setText(queueModel.item(0,0)->text());
            queueModel.removeRow(0);
            vp->open(path.toUtf8().data());
        }
    }
}

void VrokMain::process()
{
    if (eq->bar_array==NULL)
        return;

    if (vis_counter==eq->getBarSetCount()){
        vis_counter=0;
    }

    float *bars = eq->bar_array + eq->getBarCount()*vis_counter;
    for (unsigned b=0;b<eq->getBarCount();b++){
        if (bar_vals[b] > 100.0f)
            bar_vals[b]= 100.0f;
        else if (bar_vals[b] < bars[b])
            bar_vals[b] = bars[b];
        else if (bar_vals[b] < 10.0f)
            bar_vals[b] = 0.0f;
        else
            bar_vals[b] -= 8.0f;
        float gmhigh=eq->getMids()[b];
        float gbhigh=(eq->getBands()[b]-1.0f)*20.f+50.f;
        gmbars[b]->setRect(b*14,gmhigh*-1.0f,10, 1);
        gbbars[b]->setRect(b*14,gbhigh*-1.0f,10, 1);
        gbars[b]->setRect(b*14,0,10,bar_vals[b] *-1.0f);
    }
    gs->update();

    vis_counter++;
}
void VrokMain::on_btnPause_clicked()
{
    if (ui->btnSpec->isChecked())
        tx.stop();
    vp->pause();
}
void VrokMain::on_btnPlay_clicked()
{
    vp->play();
    if (ui->btnSpec->isChecked())
        tx.start();
}

void VrokMain::on_btnOpenDir_clicked()
{
    QString d = QFileDialog::getExistingDirectory(this, tr("Open Dir"),
                                                  QString(VSettings::getSingleton()->readString("lastopen","").c_str()),
                                                  0);

    if (!d.isEmpty()) {
        VSettings::getSingleton()->writeString("lastopen",d.toStdString());
        dirFilesModel.clear();
        lblDisplay->setText("Scanning...");
        qApp->processEvents();
        FolderSeeker::getSingleton()->setSeekPath(d);
        lblDisplay->setText("Done.");
        qApp->processEvents();
        ui->sbFolderSeek->setMinimum(0);
        ui->sbFolderSeek->setMaximum(FolderSeeker::getSingleton()->getFolderCount()-1);
        FolderSeeker::getSingleton()->getQueue(&dirFilesModel,0);
        on_sbFolderSeek_valueChanged(0);
        PlaylistFactory::getSingleton()->setRoot(d);
    }
}

void VrokMain::on_lvFiles_doubleClicked(QModelIndex i)
{
    vp->open(dirFilesModel.item(i.row(),1)->text().toUtf8().data());
    lblDisplay->setText(dirFilesModel.item(i.row(),0)->text());
    if (ui->btnSpec->isChecked()) {
        tx.start();
    }
}

void VrokMain::on_btnEQ_clicked()
{
    if (ew)
        delete ew;
    ew = new EQWidget(&dockManager,eq);
    ew->registerUi();
 // this->addDockWidget(Qt::BottomDockWidgetArea,ew);


}
void VrokMain::on_btnEQt_clicked()
{
    if (vp->isEffectActive((VPEffectPlugin *)eq)){
        VSettings::getSingleton()->writeInt("eqon",0);
        vp->removeEffect((VPEffectPlugin *)eq);
    } else {
        vp->addEffect((VPEffectPlugin *)eq);
        VSettings::getSingleton()->writeInt("eqon",1);
    }
}
void VrokMain::on_btnSpec_clicked()
{
    if (vw)
        delete vw;
    vw = new VSWidget(&dockManager,NULL);
    vw->registerUi();
   // this->addDockWidget(Qt::BottomDockWidgetArea,vw);


   // dockManager.tabify();
    /*
    if (vp->isEffectActive((VPEffectPlugin *)eq)){
        if (ui->btnSpec->isChecked())
            tx.start();
        else
            tx.stop();
    }
*/
}
VrokMain::~VrokMain()
{
    // effect plugins are cleaned by VPlayer
    if (vp)
        delete vp;
    if (eq)
        delete eq;
    if (gs)
        delete gs;

    delete ui;
}

void VrokMain::on_sbFolderSeek_valueChanged(int value)
{
    if (FolderSeeker::getSingleton()->getFolderCount() > 0) {

        QString dname=FolderSeeker::getSingleton()->getQueue(&dirFilesModel,value);
        lblDisplay->setText(dname.section('/',-1,-1));
    }
}

void VrokMain::actionQueueTriggered()
{

    QModelIndexList selected=ui->lvFiles->selectionModel()->selectedIndexes();
    for (int i=0;i<selected.size();i++) {

        int r = queueModel.rowCount();
        queueModel.setItem(r,0,dirFilesModel.item(selected[i].row(),0)->clone() );
        queueModel.setItem(r,1,dirFilesModel.item(selected[i].row(),1)->clone() );

    }
}

void VrokMain::actionQueueRemove()
{
    QModelIndexList selected=ui->lvQueue->selectionModel()->selectedIndexes();
    if (selected.size()  > 0) {
        queueModel.removeRows(selected.first().row(),selected.size());
    }
}

void VrokMain::actionNewQueue()
{
    ui->tbQueues->addTab( new QWidget(),"Queue " + QString::number(ui->tbQueues->count()+1));
}


QStringList VrokMain::getExtentionsList()
{
    std::vector<std::string> exts;
    QStringList list;
    vp->getSupportedFileTypeExtensions(exts);


    for (int i=0;i<(int)exts.size();i++) {
        list.append(QString("*.").append(exts[i].c_str()));
    }
    return list;
}


void VrokMain::on_lvQueue_doubleClicked(const QModelIndex &index)
{

    QString path = queueModel.item(index.row(),1)->text();
    lblDisplay->setText(queueModel.item(index.row(),0)->text());
    queueModel.removeRow(index.row());
    vp->open(path.toUtf8().data());

}

void VrokMain::on_tbQueues_tabCloseRequested(int index)
{
    if (index == 0) {
        QMessageBox( QMessageBox::Warning,"Error","Can't remove default playlist").exec();
    } else {
        ui->tbQueues->removeTab(index);
    }
}

void VrokMain::on_tbQueues_currentChanged(int index)
{
    DBG(index);

    PlaylistFactory::getSingleton()->saveQueue(&queueModel,lastTab);
    PlaylistFactory::getSingleton()->loadQueue(&queueModel,index);
    lastTab = index;

}

void VrokMain::on_sbPosition_sliderReleased()
{
    if (vp->isPlaying())
        vp->setPosition(ui->sbPosition->value()/1000.0f);
}
