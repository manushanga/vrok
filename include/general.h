#ifndef GENERAL_H
#define GENERAL_H
#include "vplayer.h"
#include "vrok.h"

class VPGeneralPlugin
{
protected:
    VPlayer *owner;
public:
    virtual ~VPGeneralPlugin() {}
};
#endif // GENERAL_H
