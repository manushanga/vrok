#include <cstring>
#include "config.h"
#include "vputils.h"

static QSettings settings("MX Ent.","Vrok");

void config_init()
{
    DBG("config: up");
    if (!settings.contains("general/volume")) {
        DBG("config: setting defaults");
        settings.setValue("general/volume", 1.0f);
        settings.setValue("user/lastopen", "");

        settings.beginGroup("general");
        settings.beginWriteArray("eq",18);
        for (int i=0;i<18;i++){
            settings.setArrayIndex(i);
            settings.setValue("gain",1.0f);
        }
        settings.endArray();
        settings.endGroup();
        settings.setValue("general/eq/preamp",QVariant(1.0f));
    }
}

QString config_get_lastopen()
{
    return settings.value("user/lastopen").toString();
}

void config_set_lastopen(QString last)
{
    settings.setValue("user/lastopen",last);
}
void config_set_eq_bands(float *bands)
{
    settings.beginGroup("general");
    settings.beginWriteArray("eq",18);
    for (int i=0;i<18;i++){
        settings.setArrayIndex(i);
        settings.setValue("gain",bands[i]);
    }
    settings.endArray();
    settings.endGroup();
}
void config_set_eq_preamp(float pa)
{
    settings.setValue("general/eq/preamp",QVariant(pa));
}
float config_get_eq_preamp()
{
    settings.value("general/eq/preamp").toFloat();
}

void config_get_eq_bands(float *bands)
{
    settings.beginGroup("general");
    settings.beginReadArray("eq");
    for (int i=0;i<18;i++){
        settings.setArrayIndex(i);
        bands[i]=settings.value("gain").toFloat();
    }
    settings.endArray();
    settings.endGroup();
}
float config_get_volume()
{
    return settings.value("general/volume").toFloat();
}
void config_set_volume(float vol)
{
    settings.setValue("general/volume", vol);
}

void config_finit()
{
    DBG("config: dying");
    settings.sync();
}
