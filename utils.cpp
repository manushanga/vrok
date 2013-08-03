#include "vputils.h"
#include "threads.h"

#ifdef DEBUG
std::mutex __m_dbgw;
#endif

uint FNV(char *str)
{
    uint hash = 2166136261;
    while (*str) {
        hash = (hash * 16777619) ^ ((unsigned)*str);
        str++;
    }
    return hash;
}
