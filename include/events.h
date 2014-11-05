#ifndef EVENTS_H
#define EVENTS_H

#include "threads.h"

#include <map>
#include <vector>
#include <string>
#include <list>
#include <ctime>
#include <vector>
#include <queue>

class VPEvents{
public:
    typedef void (*VPListener)(void *message, int messageLength, void *user);
    typedef void (*VPJob)(void *user);
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
    struct VPJobMetadata{
        VPJob job;
        void *user;
        int timeslices;
    };
    std::mutex mutex_;
    std::map < std::string , VPEvent > events_;
    std::list<VPJobMetadata> jobs_;
    std::thread *jobs_thread_;
    std::mutex jobs_guard_;
    std::queue<VPJobMetadata> jobs_add_q, jobs_rem_q;

    bool jobs_work;
    static void run_jobs(void *user);
public:
    static VPEvents *getSingleton();
    VPEvents();
    void schedulerAddJob(VPJob job, void *user, int timeslices);
    void schedulerRemoveJob(VPJob *job);

    void addEvent(std::string eventName, int limit);
    void addListener(std::string event, VPListener listener, void *user);
    void removeListener(std::string event, VPListener listener);
    void removeEvent(std::string event);
    VPListenerDataList *getListeners(std::string event);
    void fire(std::string event, void *message, int messageLength);
};
#endif // EVENTS_H
