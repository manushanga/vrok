#include <cstring>

#include "config.h"
#include "vputils.h"

static QSettings settings("MX Ent.","Vrok");

void config_init()
{
    DBG("config: up");
    if (!settings.contains("plugins/out")) {
        DBG("config: setting defaults");
        settings.setValue("plugins/out", DEFAULT_VPOUT_PLUGIN);
        settings.setValue("general/volume", 1.0f);
    }
}

float config_get_volume()
{
    return settings.value("general/volume").toFloat();
}
void config_set_volume(float vol)
{
    settings.setValue("general/volume", vol);
}
creator_t config_get_VPOutPlugin_creator()
{
    const char *set = settings.value("plugins/out").toString().toStdString().c_str();
    DBG(set);
    for (unsigned i=0;i<sizeof(vpout_entries)/sizeof(vpout_entry_t);i++){
        if (strcmp(vpout_entries[i].name, set) == 0) {
            DBG("config: VPOutPlugin: "<<vpout_entries[i].name);
            return vpout_entries[i].creator;
        }
    }
    return NULL;
}

void config_finit()
{
    DBG("config: dying");
    settings.sync();
}
