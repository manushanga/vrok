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
    mutex()
    {
        m_counter = 0;
        m_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
    }

    ~mutex()
    {
        CloseHandle(m_semaphore);
    }

    void lock()
    {
        if (InterlockedIncrement(&m_counter) > 1) // x86/64 guarantees acquire semantics
        {
            WaitForSingleObject(m_semaphore, INFINITE);
        }
    }

    void unlock()
    {
        if (InterlockedDecrement(&m_counter) > 0) // x86/64 guarantees release semantics
        {
            ReleaseSemaphore(m_semaphore, 1, NULL);
        }
    }
    bool try_lock()
    {
        LONG result = InterlockedCompareExchange(&m_counter, 1, 0);
        return (result != 0);
    }
};
}
#elif defined(__linux__)
//#include <thread>
//#include <mutex>
#include <pthread.h>
#include <semaphore.h>
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
    void detach()
    {
        pthread_detach(th);
    }
    void join()
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
    sem_t m_semaphore;

public:
    mutex()
    {
        sem_init(&m_semaphore,0,0);
    }

    ~mutex()
    {
        sem_close(&m_semaphore);
    }

    void lock()
    {
        sem_wait(&m_semaphore);
    }

    void unlock()
    {
        sem_post(&m_semaphore);
    }
    bool try_lock()
    {
        return (sem_trywait(&m_semaphore) != 0);
    }
};
}
#else
#error "Unsupported platform"
#endif

#endif // THREADS_H
