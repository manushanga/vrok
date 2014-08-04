
#include "events.h"

VPEvents *VPEvents::getSingleton()
{
    static VPEvents vpe;
    return &vpe;
}

void VPEvents::addEvent(std::string eventName, int limit)
{
    std::vector< VPListenerData > listeners;
    VPEvent event;
    event.limit = limit;
    event.list = VPListenerDataList();
    mutex_.lock();
    events_.insert(std::pair<std::string, VPEvent >(eventName,event));
    mutex_.unlock();
}

void VPEvents::addListener(std::string event, VPListener listener, void *user)
{
    mutex_.lock();
    std::map< std::string, VPEvent >::iterator it;
    it=events_.find(event);
    if (it != events_.end()){
        VPListenerData data;
        data.listener = listener;
        data.user = user;
        it->second.list.push_back(data);
    }
    mutex_.unlock();
}

void VPEvents::removeListener(std::string event, VPEvents::VPListener listener)
{
    mutex_.lock();
    std::map< std::string, VPEvent >::iterator it;
    it=events_.find(event);
    if (it != events_.end()){
        for (VPListenerDataList::iterator it_listener = it->second.list.begin();
             it_listener!=it->second.list.end();
             it_listener++)
        {
            if (it_listener->listener == listener){
                it->second.list.erase(it_listener);
                break;
            }
        }
    }
    mutex_.unlock();
}

void VPEvents::removeEvent(std::string event)
{
    mutex_.lock();
    events_.erase(event);
    mutex_.unlock();
}

VPEvents::VPListenerDataList *VPEvents::getListeners(std::string event)
{
    std::map< std::string, VPEvent >::iterator it;

    mutex_.lock();
    it=events_.find(event);

    mutex_.unlock();
    if (it != events_.end())
        return &(it->second.list);
    else
        return NULL;
}

void VPEvents::fire(std::string event, void *message, int messageLength)
{
    mutex_.lock();
    int i=0;
    std::map< std::string, VPEvent >::iterator it;
    it=events_.find(event);
    if (it != events_.end()){
        for (VPListenerDataList::iterator it_listener = it->second.list.begin();
             it_listener!=it->second.list.end();
             it_listener++)
        {
            if (it->second.limit > 0 && i>it->second.limit)
                break;
            VPListener listener =(VPListener ) it_listener->listener;
            //DBG(std::string("firing ")<<event<<" for "<<(void*)listener);
            listener(message, messageLength, it_listener->user);
            i++;
        }
    }
    mutex_.unlock();
}
