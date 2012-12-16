#ifndef VPUTILS_H
#define VPUTILS_H

//#define DEBUG
#ifdef DEBUG
    #include <iostream>
    #define DBG(...) std::cout<<"DBG "<<__VA_ARGS__<<std::endl
#else
    #define DBG(...)
#endif

#endif // VPUTILS_H
