#ifndef THREADS_H
#define THREADS_H
#if defined( __GNUC__) || defined(__clang__)
#define ATOMIC_CAS(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)
#define ATOMIC_FADD(ptr,v) __sync_fetch_and_add(ptr, v)
#define ATOMIC_FSUB(ptr,v) __sync_fetch_and_sub(ptr, v)
#define ATOMIC_ADDF(ptr,v) __sync_add_and_fetch(ptr, v)
#define ATOMIC_SUBF(ptr,v) __sync_sub_and_fetch(ptr, v)
#define ATOMIC_BOOL_CAS(ptr,oldv,newv) __sync_bool_compare_and_swap(ptr, oldv, newv)

#define FULL_MEMORY_BARRIER __sync_synchronize()

#elif defined(_MSC_VER)
// bool is implemented as 8 bits
#define ATOMIC_CAS(ptr,oldv,newv) _InterlockedCompareExchange8((char*)ptr, (char)newv, (char)oldv)

#define FULL_MEMORY_BARRIER MemoryBarrier()

/*#define ATOMIC_FADD(ptr,v) __sync_fetch_and_add(ptr, v)
#define ATOMIC_FSUB(ptr,v) __sync_fetch_and_sub(ptr, v)
#define ATOMIC_ADDF(ptr,v) __sync_add_and_fetch(ptr, v)
#define ATOMIC_SUBF(ptr,v) __sync_sub_and_fetch(ptr, v)
#define ATOMIC_BOOL_CAS(ptr,oldv,newv) __sync_bool_compare_and_swap(ptr, oldv, newv)
*/
#endif
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <process.h>

namespace std{

class thread
{
private:
    HANDLE th;
    DWORD id;

public:
    void(*runner)(void *);
    void *data;
    static DWORD WINAPI _std_thread_run(void *self)
    {
        ((std::thread*)self)->runner(((std::thread*)self)->data);
        return 0;
    }
    thread(void(*addr)(void *), void *user)
    {
        data = user;
        runner = addr;
        th = CreateThread(NULL, 100, _std_thread_run, this ,0,&id);
    }
    void detach()
    {

    }
    void join()
    {
        WaitForSingleObject(th, INFINITE);
    }
    void high_priority()
    {
        SetThreadPriority(th,THREAD_PRIORITY_ABOVE_NORMAL);
    }
    ~thread()
    {
        CloseHandle(th);
    }
};

// The below code was used for this implementation
// http://preshing.com/20111124/always-use-a-lightweight-mutex


class shared_mutex
{
private:
    LONG m_counter;
    HANDLE m_semaphore;

public:
    inline shared_mutex()
    {
        m_counter = 0;
        m_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    }

    inline ~shared_mutex()
    {
        CloseHandle(m_semaphore);
    }

    inline void lock()
    {
        if (InterlockedIncrement(&m_counter) > 1) // x86/64 guarantees acquire semantics
        {
            WaitForSingleObject(m_semaphore, INFINITE);
        }
    }

    inline void unlock()
    {
        if (InterlockedDecrement(&m_counter) > 0) // x86/64 guarantees release semantics
        {
            ReleaseSemaphore(m_semaphore, 1, NULL);
        }
    }
    inline bool try_lock()
    {
        LONG result = InterlockedCompareExchange(&m_counter, 1, 0);
        return (result == 0);
    }
};
}
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>
#define SPIN_MAX 10000

namespace std{
class thread{
private:
    pthread_t th;

public:
    void(*runner)(void *);
    void *data;
    static void *_std_thread_run(void *self)
    {
        ((std::thread*)self)->runner(((std::thread*)self)->data);
        return NULL;
    }
    thread(void(*addr)(void *), void *user)
    {
        data = user;
        runner = addr;

        pthread_create(&th,NULL,_std_thread_run, this);
    }
    void high_priority()
    {
        int ret=0;
        struct sched_param params;
        params.sched_priority = sched_get_priority_max(SCHED_FIFO);

        ret = pthread_setschedparam(th, SCHED_FIFO, &params);
        if (ret != 0) {
            cout<<"Thread can't set real-time priority."<<endl;
        }
        int policy = 0;
        ret = pthread_getschedparam(th, &policy, &params);
        if (ret != 0) {
            cout << "Couldn't retrieve real-time scheduling paramers" << endl;
            return;
        }

        if(policy != SCHED_FIFO) {
            cout << "Scheduling is NOT SCHED_FIFO!" << endl;
            return;
        }
    }
    inline void detach()
    {
        pthread_detach(th);
    }
    inline void join()
    {
        pthread_join(th, NULL);
    }
    ~thread()
    {
    }
};


class shared_mutex
{
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool signal;
public:
    shared_mutex():
        signal(true)
    {
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&cond,NULL);

    }

    ~shared_mutex()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);

    }

    inline void lock()
    {
        pthread_mutex_lock(&mutex);
        while (!signal)
            pthread_cond_wait(&cond,&mutex);
        signal=false;
        pthread_mutex_unlock(&mutex);

    }

    inline void unlock()
    {
        pthread_mutex_lock(&mutex);

        if (!signal){
            pthread_cond_signal(&cond);
        }
        signal=true;
        pthread_mutex_unlock(&mutex);

    }
    inline bool try_lock()
    {
        pthread_mutex_lock(&mutex);
        if (signal) {
            signal=false;
            pthread_mutex_unlock(&mutex);
            return true;
        } else {
            pthread_mutex_unlock(&mutex);
            return false;
        }
    }
};
}
#else
#error "Unsupported platform"
#endif

#endif // THREADS_H
