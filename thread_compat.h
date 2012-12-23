#ifndef THREAD_COMPAT_H
#define THREAD_COMPAT_H

#ifdef _WIN32
// mingw is used for compiling and this way is better, for now.
#include <boost/thread.hpp>
namespace std{
    typedef boost::thread thread;
    typedef boost::mutex mutex;
}
#elif defined(__linux__)
#include <thread>
#include <mutex>
#else
#error "Mac OS? Nope not supported, others? may be after Jack is implemented."
#endif

#endif // THREAD_COMPAT_H
