/*
  Vrok - smokin' audio
  (C) 2012 Madura A. released under GPL 2.0. All following copyrights
  hold. This notice must be retained.

  See LICENSE for details.
*/

#ifndef VPUTILS_H
#define VPUTILS_H

#define DEBUG
#ifdef DEBUG
    #include <iostream>
    #define DBG(...) std::cout<<__VA_ARGS__<<" at "<<__PRETTY_FUNCTION__<<std::endl
#else
    #define DBG(...)
#endif

#include <cstdlib>
#define vp_new(_class, _size) malloc(_size); new(_class)()
#define vp_free(_class, _obj) (_obj)->~(_class)(); free(_obj)

#endif // VPUTILS_H
