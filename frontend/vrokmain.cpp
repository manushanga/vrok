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
#include "playlistwidget.h"
#include "controlswidget.h"

#include "players/flac.h"
#include "players/mpeg.h"
#include "players/ogg.h"

#include "playlistfactory.h"

#include <cstring>
#include <ctime>

#include "orderer.h"



bool VrokMain::eventFilter(QObject *target, QEvent *event)
{

    if (event->type() == QEvent::WindowStateChange) {
        if (windowState() == Qt::WindowMinimized) {
            vp->uiStateChanged(VPMINIMIZED);
        } else if (windowState() == Qt::WindowMaximized) {
            vp->uiStateChanged(VPMAXIMIZED);
        }

        return true;
    }
    if ( event->type() == QEvent::MouseButtonPress ){
        return true;
    }
    return true;
}


void VrokMain::showAboutVrok()
{
    QDialog d(this);
    d.setWindowTitle(tr("About Vrok"));
    d.setModal(true);
    QHBoxLayout h(&d);
    d.setLayout(&h);
    QLabel a("<center>"
             "<span style=\"font-size: 12pt\"><b>Vrok 3</b> smokin' audio<br/></span>"
             "</center>"
             "Copyright (C) 2012-2014 Madura A. <madura.x86@gmail.com><br/>"
             "<br/>"
             "<b>Libraries</b><br/>"
             "<br/>Sound Ouput<br/>"
             "<a href=\"http://www.alsa-project.org\">libasound</a>: Advanced Linux Sound Architecture<br/>"
             "<a href=\"http://mega-nerd.com/SRC/\">libsamplerate</a>: Samplerate convertor(ALSA only)<br/>"
             "<a href=\"http://pulseaudio.org\">libpulse-simple</a>: PulseAudio Synchronous API<br/>"
             "<a href=\"http://msdn.microsoft.com/en-us/library/windows/desktop/ee416960%28v=vs.85%29.aspx\">DSound</a>: Windows sound output<br/>"
             "<a href=\"http://xiph.org/ao/\">libao</a>: Audio Output on supported systems<br/>"
             "<br/>Decoders<br/>"
             "<a href=\"http://flac.sourceforge.net/\">libFLAC</a>: FLAC Decoder<br/>"
             "<a href=\"http://xiph.org/vorbis/\">libvorbisfile</a>, <a href=\"http://xiph.org/ogg/\">libogg</a>: OGG Vorbis Decoder<br/>"
             "<a href=\"http://www.mpg123.de/\">libmpg123</a>: MPEG Layer 1,2,3 Decoder<br/>"
             "<a href=\"http://www.ffmpeg.org/\">libavcodec</a>: Demux & Decode AAC, MP4, MPG, AVI ... and more<br/>"
             "<br/>DSP<br/>"
             "<a href=\"http://shibatch.sourceforge.net/\">SuperEQ</a>: Naoki Shibata's 18 Band Equalizer<br/>"
             "<br/>GUI<br/>"
             "<a href=\"http://qt-project.org\">Qt</a>: Frontend<br/>"
             "<br/>"
             "<span style=\"font-size: 8pt\">Bugs may exist. Built on " __DATE__ " at " __TIME__ ".</span>" );
    h.addWidget(&a);
    d.exec();
}
VrokMain::VrokMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VrokMain),
    dockManager(new DockManager(this)),
    lastTab(0)
{
    ui->setupUi(this);
    ui->centralWidget->hide();

    vp=NULL;
    ew=NULL;
    vw=NULL;
    rw=NULL;
    sw=NULL;

    setWindowIcon(QIcon(":icon/vrok.png"));

    vp = new VPlayer(NULL, NULL);
    eq = new VPEffectPluginEQ();
    sp = new VPEffectPluginSpatial();
    vz = new VPEffectPluginVis();
    rb = new VPEffectPluginReverb();

    PlaylistWidget *pw=new PlaylistWidget(dockManager, vp);
    ControlsWidget *cw=new ControlsWidget(dockManager, this);
    cw->registerUi();
    pw->registerUi();

    effects.push_back(VPEffectInfo("Equalizer",eq,VSettings::getSingleton()->readInt("equalizer",1) ));
    effects.push_back(VPEffectInfo("Visualizer",vz,VSettings::getSingleton()->readInt("visualizer",0) ));
    effects.push_back(VPEffectInfo("Reverb",rb,VSettings::getSingleton()->readInt("reverb",0) ));
    effects.push_back(VPEffectInfo("Spatial Audio",sp,VSettings::getSingleton()->readInt("spatial_audio",1) ));

    std::vector<VPEffectPlugin *> ee;

    foreach (VPEffectInfo e, effects){
        if (e.enabled) {
            ee.push_back(e.ptr);
			if (e.name == "Equalizer") { docks.insert(e.name,new EQWidget(dockManager, (VPEffectPluginEQ *) e.ptr)); }
            else if (e.name == "Visualizer") {  docks.insert(e.name,new VSWidget(dockManager, (VPEffectPluginVis *) e.ptr)); }
            else if (e.name == "Reverb") { docks.insert(e.name,new ReverbWidget(dockManager, (VPEffectPluginReverb *) e.ptr)); }
            else if (e.name == "Spatial Audio") { docks.insert(e.name,new SpatialWidget(dockManager, (VPEffectPluginSpatial *) e.ptr)); }
			else {
				docks[e.name]=NULL;
			}
			if (docks[e.name]){
				docks[e.name]->registerUi();
			}
        }
    }

    vp->setEffectsList(ee);


    vis_counter = 0;




}




VrokMain::~VrokMain()
{
    delete dockManager;

    if (vp)
        delete vp;
    if (eq)
        delete eq;


    delete ui;
}



void VrokMain::configurePlugins()
{

    QStandardItemModel aa;
    int i=0;
    foreach (VPEffectInfo e, effects){
        QStandardItem *s1=new QStandardItem(QString(e.name));
        s1->setData(qVariantFromValue((void*)e.ptr));
        s1->setCheckable(true);
        s1->setCheckState(e.enabled ? Qt::Checked : Qt::Unchecked);
        aa.setItem(i,s1);
        i++;
    }
    Orderer xr(&aa);
    xr.exec();


    if (xr.okPressed) {
        foreach (VPEffectInfo e, effects){
            if (docks.find(e.name) != docks.end()) {
                delete( docks[e.name]);
            }
        }
        docks.clear();
        std::vector<VPEffectPlugin*> effs;
        effects.clear();


        for (int i=0;i<aa.rowCount();i++){
            VPEffectPlugin *p=(VPEffectPlugin *) xr.model->item(i,0)->data().value<void*>();

            if (xr.model->item(i,0)->checkState() == Qt::Checked) {
                VSettings::getSingleton()->writeInt(xr.model->item(i,0)->text().toLower().replace(' ','_').toStdString(),1);
                effects.push_back( VPEffectInfo(xr.model->item(i)->text() , p , true) );

                QString name=xr.model->item(i)->text();
                DBG(name.toStdString());
                if (name == "Equalizer") { docks.insert(name,new EQWidget(dockManager, (VPEffectPluginEQ *) p)); }
                else if (name == "Visualizer") { docks.insert(name,new VSWidget(dockManager, (VPEffectPluginVis *) p)); }
                else if (name == "Reverb") { docks.insert(name,new ReverbWidget(dockManager, (VPEffectPluginReverb *)p)); }
                else if (name == "Spatial Audio") { docks.insert(name,new SpatialWidget(dockManager, (VPEffectPluginSpatial *) p)); }
                else {
                    docks[name]=NULL;
                }
                if (docks[name]){
                    docks[name]->registerUi();
                }

                effs.push_back(p);
            } else {
                VSettings::getSingleton()->writeInt(xr.model->item(i,0)->text().toLower().replace(' ','_').toStdString(),0);
                //DBG("xx"<<xr.model->item(i)->text().toStdString());

                effects.push_back( VPEffectInfo(xr.model->item(i)->text() , p , false) );
            }
        }

        vp->setEffectsList(effs);
    }


}
