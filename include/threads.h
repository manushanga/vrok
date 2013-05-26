#ifndef THREADS_H
#define THREADS_H

#ifdef _WIN32
#include <windows.h>
#include <process.h>

#define ATOMIC_CAS(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)

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
        th = CreateThread(NULL, 10, _std_thread_run, this ,0,&id);
    }
    void detach()
    {

    }
    void join()
    {
        WaitForSingleObject(th, INFINITE);
    }
    ~thread()
    {
        CloseHandle(th);
    }
};

// The below code was used for this implementation
// http://preshing.com/20111124/always-use-a-lightweight-mutex


class mutex
{
private:
    LONG m_counter;
    HANDLE m_semaphore;

public:
    inline mutex()
    {
        m_counter = 0;
        m_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    }

    inline ~mutex()
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
        return (result != 0);
    }
};
}
#elif defined(__linux__)
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#define SPIN_MAX 100

#define ATOMIC_CAS(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)
#define ATOMIC_ADD(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)
#define ATOMIC_SUB(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)
#define ATOMIC_BOOL_CAS(ptr,oldv,newv) __sync_bool_compare_and_swap(ptr, oldv, newv)

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
        pthread_exit(0);
    }
    thread(void(*addr)(void *), void *user)
    {
        data = user;
        runner = addr;

        pthread_create(&th,NULL,_std_thread_run, this);
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


class mutex
{
private:
    volatile int avail __attribute__((aligned(16))) ;
    volatile int spin;
    volatile int waiters;
    volatile int cs;
public:
    enum State { UNLOCKED=0, LOCKED=1, CONTENDED=2 };
    inline mutex()
    {
        cs=UNLOCKED;
        avail = 1;
        waiters = 0;
        spin = 1;
    }

    inline ~mutex()
    {

    }

    inline void lock()
    {
        for (unsigned int i=0;i<SPIN_MAX;i++)
        {
            if (__sync_bool_compare_and_swap (&cs,UNLOCKED,LOCKED)) {
                return;
            }
        }

        __sync_bool_compare_and_swap(&cs,LOCKED,CONTENDED);

        int val;
        while (1) {
            val = avail;
            if( val > 0 && __sync_bool_compare_and_swap(&avail, val, val - 1) )
                break;

            __sync_fetch_and_add(&waiters, 1);
            syscall(__NR_futex, &avail, FUTEX_WAIT_PRIVATE, val, NULL, 0, 0);
            __sync_fetch_and_sub(&waiters, 1);

        }
        if ( waiters == 0 ){
            __sync_bool_compare_and_swap(&cs,CONTENDED,LOCKED);
        }
    }

    inline void unlock()
    {
        if (__sync_bool_compare_and_swap(&cs,LOCKED,UNLOCKED)) {
            return;
        } else if (__sync_bool_compare_and_swap(&cs,CONTENDED,CONTENDED)) {
            int nval = __sync_add_and_fetch(&avail, 1);

            if( waiters > 0 ) {
                syscall(__NR_futex, &avail, FUTEX_WAKE_PRIVATE, nval, NULL, 0, 0);
            }
        }

    }
    inline bool try_lock()
    {
        int val = avail;
        if( val > 0 ) {
            lock();
            return true;
        } else {
            return false;
        }
    }
};
}
#else
#error "Unsupported platform"
#endif

#endif // THREADS_H
