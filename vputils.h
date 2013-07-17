/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef VPUTILS_H
#define VPUTILS_H
#include <sstream>

#define DEBUG

#ifdef DEBUG
    #include <iostream>
    #include "threads.h"
    extern std::mutex __m_dbgw;
    #define DBG(...) \
    __m_dbgw.lock(); \
    std::cout<<__VA_ARGS__<<" at "<<__PRETTY_FUNCTION__<<std::endl; \
    __m_dbgw.unlock();
#else
    #define DBG(...)
#endif

#define strtolower(p) for ( ; *p; ++p) *p = tolower(*p)


#define TOSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#endif // VPUTILS_H
