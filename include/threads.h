#ifndef THREADS_H
#define THREADS_H

#define ATOMIC_CAS(ptr,oldv,newv) __sync_val_compare_and_swap(ptr, oldv, newv)
#define ATOMIC_FADD(ptr,v) __sync_fetch_and_add(ptr, v)
#define ATOMIC_FSUB(ptr,v) __sync_fetch_and_sub(ptr, v)
#define ATOMIC_ADDF(ptr,v) __sync_add_and_fetch(ptr, v)
#define ATOMIC_SUBF(ptr,v) __sync_sub_and_fetch(ptr, v)
#define ATOMIC_BOOL_CAS(ptr,oldv,newv) __sync_bool_compare_and_swap(ptr, oldv, newv)

#ifdef _WIN32
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
#include <semaphore.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/futex.h>
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
        pthread_exit(0);
    }
    thread(void(*addr)(void *), void *user)
    {
        data = user;
        runner = addr;

        pthread_create(&th,NULL,_std_thread_run, this);
        std::cout<<"thread create"<<std::endl;
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
    volatile int cs;
    sem_t sem;
public:
    inline mutex()
    {
        sem_init(&sem,0,1);
    //    pthread_mutex_init(&m_mutex,NULL);
        cs=0;
    }

    inline ~mutex()
    {
    //    pthread_mutex_destroy(&m_mutex);
    }

    inline void lock()
    {
        sem_wait(&sem);
  //      pthread_mutex_lock(&m_mutex);
/*
        int i, c;

        for (i = 0; i < SPIN_MAX; i++)
        {
            c = __sync_val_compare_and_swap(&cs, 0, 1);
            if (!c)
                return;

        }

        if (c == 1) {
            c = __sync_lock_test_and_set(&cs,2);
        }

        while (c)
        {
            syscall(__NR_futex, &cs, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
            c = __sync_lock_test_and_set(&cs, 2);
        }
*/
    }

    inline void unlock()
    {
        sem_post(&sem);
//        pthread_mutex_unlock(&m_mutex);
/*
        int i;


        if (cs == 2)
        {
            __sync_lock_test_and_set(&cs, 0) ;
        }
        else if (__sync_lock_test_and_set(&cs, 0) == 1)
            return ;

        for (i = 0; i < SPIN_MAX; i++)
        {
            if (cs)
            {

                if (__sync_val_compare_and_swap(&cs, 1, 2))
                    return ;
            }
        }

        syscall(__NR_futex, &cs, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
*/
    }
    inline bool try_lock()
    {
        return (bool) (sem_trywait(&sem)==0);
        //return pthread_mutex_trylock(&m_mutex);
/*
        int c = __sync_val_compare_and_swap(&cs, 0, 1);
        if (!c)
            return true;
        return false;*/
    }
};
}
#else
#error "Unsupported platform"
#endif

#endif // THREADS_H
