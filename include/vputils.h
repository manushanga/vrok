/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef VPUTILS_H
#define VPUTILS_H
#include <sstream>
#include "threads.h"
extern std::mutex __m_console;

#define DEBUG

#ifdef DEBUG
    #include <iostream>
    #define DBG(...) \
    __m_console.lock(); \
    std::cout<<__VA_ARGS__<<" at "<<__PRETTY_FUNCTION__<<std::endl; \
    __m_console.unlock();
#else
    #define DBG(...)
#endif
#define WARN(...) \
    __m_console.lock(); \
    std::cerr<<__VA_ARGS__<<std::endl; \
    __m_console.unlock();


#include <cstdio>
#if defined(_WIN32) && defined(UNICODE)
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
