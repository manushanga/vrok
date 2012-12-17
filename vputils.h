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
    #define DBG(...) std::cout<<"DBG "<<__VA_ARGS__<<std::endl
#else
    #define DBG(...)
#endif

#endif // VPUTILS_H
