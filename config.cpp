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
