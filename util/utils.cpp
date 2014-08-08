#include "vputils.h"
#include "threads.h"

std::shared_mutex __m_console;
#ifdef LOG_TO_FILE
std::ofstream __m_log;
GlobalInitialization __m_global;
#endif

#ifdef LOG_TO_FILE
#ifdef _WIN32
#include <shlobj.h>
#include <shlwapi.h>
static std::string __get_temp_path()
{
    TCHAR szPath[MAX_PATH];

    if(SUCCEEDED(SHGetFolderPath(NULL,
                                 CSIDL_APPDATA,
                                 NULL,
                                 0,
                                 szPath)))
    {
        FILE *f;
        PathAppend(szPath,TEXT("vrok"));
        CreateDirectory(szPath,NULL);
        PathAppend(szPath,TEXT("log.txt"));
        f=_wfopen(szPath,TEXT("r"));

        char upath[1024];
        WideCharToMultiByte(CP_UTF8,0,szPath,-1,upath,1024,0,NULL);
        return std::string(upath);
    } else {
        exit(-1);
    }
}
#elif defined(__linux__)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
static std::string __get_temp_path()
{
    char *home = getenv("HOME");
    std::string path;
    path.append(home);
    path.append("/.config/MXE");

    struct stat buf;
    int ret=stat(path.c_str(),&buf);
    if (ret) {
        mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    path.append("/log.txt");
    return path;
}
#else
    #error "Unsupported platform!"
#endif

GlobalInitialization::GlobalInitialization(){
    __m_log.open(__get_temp_path().c_str(),std::ios::out);
}
GlobalInitialization::~GlobalInitialization(){
    __m_log.close();
}

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


