#ifndef THREADS_H
#define THREADS_H

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
    pthread_mutex_t m_mutex;
public:
    inline mutex()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }

    inline ~mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    inline void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    inline void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }
    inline bool try_lock()
    {
        return ( pthread_mutex_trylock(&m_mutex) != 0);
    }
};
}
#else
#error "Unsupported platform"
#endif

#endif // THREADS_H
