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

//
// stackoverflow.com/questions/1666802/is-there-a-class-macro-in-c
//
std::string function_name(const std::string& func)
{
    size_t colons = func.find("::");
    return func.substr(colons+2);
}

std::string class_name(const std::string& func)
{
    size_t colons = func.find("::");
    if (colons == std::string::npos)
        return "::";
    size_t begin = func.substr(0,colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return func.substr(begin,end);

}


