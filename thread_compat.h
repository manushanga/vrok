/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/
#ifndef THREAD_COMPAT_H
#define THREAD_COMPAT_H

#ifdef _WIN32
// mingw is used for compiling and this way is better, for now.

#include <windows.h>
#include <process.h>
// This is my own ugly implementation for Windows, and it replaces
// boost's need in this project

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
      //  Sleep(2);
    }

    void unlock()
    {
        if (InterlockedDecrement(&m_counter) > 0) // x86/64 guarantees release semantics
        {
            ReleaseSemaphore(m_semaphore, 1, NULL);
        }
       // Sleep(2);
    }
    bool try_lock()
    {
        LONG result = InterlockedCompareExchange(&m_counter, 1, 0);
        return (result != 0);
    }
};
}

#elif defined(__linux__)
#include <thread>
#include <mutex>
#else
#error "Mac OS? Nope not supported, others? may be after Jack is implemented."
#endif

#endif // THREAD_COMPAT_H
