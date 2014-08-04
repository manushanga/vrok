#ifndef EVENTS_H
#define EVENTS_H

#include "threads.h"

#include <map>
#include <vector>
#include <string>
class VPEvents{
public:
    typedef void (*VPListener)(void *message, int messageLength, void *user);
    struct VPListenerData{
        VPListener listener;
        void *user;
    };
    typedef std::vector < VPListenerData > VPListenerDataList;
private:
    struct VPEvent{
        int limit;
        VPListenerDataList list;
    };
    std::mutex mutex_;
    std::map < std::string , VPEvent > events_;
public:
    static VPEvents *getSingleton();
    void addEvent(std::string eventName, int limit);
    void addListener(std::string event, VPListener listener, void *user);
    void removeListener(std::string event, VPListener listener);
    void removeEvent(std::string event);
    VPListenerDataList *getListeners(std::string event);
    void fire(std::string event, void *message, int messageLength);
};
#endif // EVENTS_H
