
#include "events.h"

void VPEvents::run_jobs(void *user)
{

    VPEvents *self=(VPEvents*)user;
    std::list<VPEvents::VPJobMetadata>::iterator it=self->jobs_.begin(),it1;
    while (ATOMIC_CAS(&self->jobs_work,true,true)) {

        if (self->jobs_.size() > 0) {
            time_t start=time(NULL);
            //std::cout<<"c"<<self->jobs_.size()<<" "<<it->job<<std::endl;
            (it->job)(it->user);
            time_t delta=time(NULL)-start;
            if (delta < 5) {
                sleep(5 - delta);
            }

            it++;
            if (it==self->jobs_.end()){
                it=self->jobs_.begin();
            }
        } else {
            sleep(3);
        }
        self->jobs_guard_.lock();
        while (self->jobs_add_q.size() >0) {
            self->jobs_.push_back( self->jobs_add_q.front() );
            self->jobs_add_q.pop();
        }
        /*while (self->jobs_rem_q.size() >0) {
            for (it1=self->jobs_.begin();
                 it1!=self->jobs_.end();
                 it1++) {

                self->jobs_.remove(self->jobs_rem_q.front());
            }
            self->jobs_rem_q.pop();
        }*/

        it=self->jobs_.begin();
        self->jobs_guard_.unlock();
    }

}

VPEvents *VPEvents::getSingleton()
{
    static VPEvents vpe;
    return &vpe;
}

VPEvents::VPEvents()
{
    jobs_work=true;
    jobs_thread_ = new std::thread(VPEvents::run_jobs, this);

}

void VPEvents::schedulerAddJob(VPEvents::VPJob job, void *user, int timeslices)
{
    jobs_guard_.lock();
    VPJobMetadata jb;
    jb.job=job;
    jb.user=user;
    jb.timeslices=timeslices;
    jobs_add_q.push(jb);
    jobs_guard_.unlock();
}

void VPEvents::schedulerRemoveJob(VPEvents::VPJob *job)
{
   /* jobs_guard_.lock();
    VPJobMetadata jb;
    jb.job=job;
    jb.user=user;
    jb.timeslices=timeslices;
    jobs_rem_q.push(jb);
    jobs_guard_.unlock();
    */
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
