/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef VPUTILS_H
#define VPUTILS_H
#include <sstream>
#include <cstdlib>
#include "threads.h"

#if __WORDSIZE == 64
#define CPU64
#define WORDSIZE 64
#elif __WORDSIZE == 32
#define CPU32
#define WORDSIZE 32
#else

#if _WIN64
#define WORDSIZE 64
#define CPU64
#elif _WIN32
#define WORDSIZE 32
#define CPU32
#endif

#endif

extern std::shared_mutex __m_console;

// aligning
#if defined(__GNUC__)
#define ALIGNAUTO(x) x __attribute__((aligned(sizeof(void *)*2)))
#define ALIGN(x,y) x __attribute__((aligned(y)))

// x should be a multiple of sizeof(void *)*2, which is 16 or 8 in most known
// systems; you are guaranteed that this holds if you allocate multiples of
// VPBUFFER_FRAMES or VPBUFFER_PERIOD(==512)
#define ALIGNED_ALLOC(x) aligned_alloc(sizeof(void *)*2, (x))
#define ALIGNED_FREE(x) free(x)
#elif _MSC_VER
#define ALIGNAUTO(x) __declspec( align( 16 ) ) x
#define ALIGN(x,y) __declspec( align( y ) ) x
#define ALIGNED_ALLOC(x) _aligned_malloc((x),sizeof(void *)*2)
#define ALIGNED_FREE(x) _aligned_free(x)
#endif

#if _MSC_VER
    #define FUNCTION_NAME __FUNCTION__
#elif __GNUC__
    #define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#define DEBUG 
#if defined(DEBUG) || !defined(NDEBUG)
    #include <iostream>
    #define DBG(...) \
    __m_console.lock(); \
    std::cout<<__VA_ARGS__<<" at "<<FUNCTION_NAME<<std::endl; \
    __m_console.unlock();
#else
    #define DBG(...)
#endif

#if WIN32 || WIN64
#include <WinUser.h>
#include <sstream>
#include <string>
#define WARN(...)  \
	{ \
		std::stringstream ss; \
		ss<<__VA_ARGS__; \
		MessageBoxA(GetForegroundWindow(),ss.str().c_str(),"",MB_OK); \
	}
#else
#define WARN(...) \
    __m_console.lock(); \
    std::cerr<<__VA_ARGS__<<std::endl; \
    __m_console.unlock();

#endif

#if defined(_WIN32) && defined(UNICODE)
#include <cstdio>
#include <winnls.h>
inline FILE *fopenu(const char *path,const char *opt){
    DBG(path);
    wchar_t wpath[1024];
    wchar_t wopt[8];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 1024);
    MultiByteToWideChar(CP_UTF8, 0, opt, -1, wopt, 8);

    return _wfopen(wpath, wopt);
}
#define FOPEN_RB "rb"
#define FOPEN_WB "wb"
#define FOPEN_AB "wb+"
#else
#define fopenu fopen
#define FOPEN_RB "r"
#define FOPEN_WB "w"
#define FOPEN_AB "w+"
#endif

typedef unsigned int uint;
uint FNV(char *str);
#endif // VPUTILS_H
