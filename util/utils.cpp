#include "vputils.h"
#include "threads.h"

std::shared_mutex __m_console;

uint FNV(char *str)
{
    uint hash = 2166136261;
    while (*str) {
        hash = (hash * 16777619) ^ ((unsigned)*str);
        str++;
    }
    return hash;
}
