#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "vrok.h"
#include "vputils.h"

#ifdef _WIN32
#include <shlobj.h>
#include <shlwapi.h>
static std::string settings_path(bool *firsttime)
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
        PathAppend(szPath,TEXT("vrok.conf"));
        f=_wfopen(szPath,TEXT("r"));
        DBG(szPath);
        if (!f) {
            *firsttime = true;
        } else {
            *firsttime = false;
            fclose(f);
        }
        char upath[1024];
        WideCharToMultiByte(CP_UTF8,0,szPath,-1,upath,1024,0,NULL);
        return std::string(upath);
    } else {
        DBG("Can't get settings path, exiting");
        exit(-1);
    }
}
#elif defined(__linux__)
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
static std::string settings_path(bool *firsttime)
{
    char *home = getenv("HOME");
    DBG("config home"<<home);
    std::string path;
    path.append(home);
    path.append("/.config/MXE");

    struct stat buf;
    int ret=stat(path.c_str(),&buf);
    if (ret) {
        mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    path.append("/vrok.conf");
    FILE *f=fopenu(path.c_str(),"r");
    if (!f) {
        *firsttime = true;
    } else {
        *firsttime = false;
        fclose(f);
    }
    return path;
}
#else
    #error "Unsupported platform!"
#endif

VSettings::VSettings()
{
    bool first;
    std::string path = settings_path(&first);
    DBG("config: up");
    if (!first) {
        FILE *f = fopenu(path.c_str(),"r");
        DBG("read");
        while (!feof(f)){
            char name[32];
            fscanf(f,"%32s",name);
            int count;
            fscanf(f,"%d",&count);

            std::vector<int> list;
            for (int i=0;i<count;i++){
                int num;
                fscanf(f,"%d",&num);
                list.push_back(num);
            }
            settings.insert(std::pair< std::string, std::vector<int> >(std::string(name),list));
        }

        fclose(f);

    }
    if ( readInt("version", -1) <  VERSION)
    {
        first=true;
        DBG("older version detected, overwriting settings");
        settings.clear();
        writeInt("version",2);
    }
}

std::string VSettings::readString(std::string field, std::string def)
{
    if (settings.find(field)!=settings.end()){
        std::vector<int>& list = settings[field];
        int n=list.size() +1;
#ifdef _MSC_VER
        char *buffer=new char[n];
#else
        char buffer[n];
#endif
        memset(buffer,'\0',list.size()+1);
        for (size_t i=0;i<list.size();i++){
            buffer[i] = (char)list[i];
        }

        std::string ss(buffer);
#ifdef _MSC_VER
        delete buffer;
#endif
        return ss;
    }  else {
        std::vector<int> list;
        unsigned char *ptr = (unsigned char *) def.c_str();
        for (size_t i=0;i<def.length();i++) {
            list.push_back((unsigned char) ptr[i]);
        }

        settings.insert(std::pair<std::string, std::vector<int> >(field, list) );

        return def;
    }
}
void VSettings::writeString(std::string field, std::string str)
{
    std::map< std::string, std::vector<int> >::iterator it = settings.find(field);
    std::vector<int> list;
    if (settings.end()== it) {
        settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = settings.find(field);
    }

    (*it).second.clear();
    for (size_t i=0;i<str.length();i++){
        (*it).second.push_back((int)str[i]);
    }
}
int VSettings::readInt(std::string field, int def)
{
    if (settings.find(field)!=settings.end()){
        std::vector<int>& list = settings[field];
        return list[0];
    } else {
        std::vector<int> list;
        list.push_back(def);
        settings.insert(std::pair<std::string, std::vector<int> >(field, list) );
        return def;
    }
}
void VSettings::writeInt(std::string field, int i)
{
    std::map< std::string, std::vector<int> >::iterator it = settings.find(field);
    std::vector<int> list;
    if (settings.end() == it) {
        settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = settings.find(field);
    }

    (*it).second.clear();
    (*it).second.push_back(i);
}
double VSettings::readDouble(std::string field, double def)
{
    if (settings.find(field)!=settings.end()){
        std::vector<int>& list = settings[field];
        double d;
        unsigned char *ptr = (unsigned char *)&d;
        for (size_t i=0;i<sizeof(double);i++) {
            ptr[i]=(unsigned char)list[i];
        }
        return d;
    } else {
        std::vector<int> list;
        unsigned char *ptr = (unsigned char *)&def;
        for (size_t i=0;i<sizeof(double);i++) {
            list.push_back((unsigned char)ptr[i]);
        }

        settings.insert(std::pair<std::string, std::vector<int> >(field, list) );
        return def;
    }

}
void VSettings::writeDouble(std::string field, double dbl)
{
    std::map< std::string, std::vector<int> >::iterator it = settings.find(field);
    std::vector<int> list;
    if (settings.end() == it) {
        settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = settings.find(field);
    }

    (*it).second.clear();
    unsigned char *ptr = (unsigned char *) &dbl;
    for (size_t i=0;i<sizeof(double);i++){
        (*it).second.push_back((int)ptr[i]);
    }
}

float VSettings::readFloat(std::string field, float def)
{
    if (settings.find(field)!=settings.end()){
        std::vector<int>& list = settings[field];
        float d;
        unsigned char *ptr = (unsigned char *)&d;
        for (size_t i=0;i<sizeof(float);i++) {
            ptr[i]=(unsigned char)list[i];
        }
        return d;
    } else {
        std::vector<int> list;
        unsigned char *ptr = (unsigned char *)&def;
        for (size_t i=0;i<sizeof(float);i++) {
            list.push_back((unsigned char)ptr[i]);
        }

        settings.insert(std::pair<std::string, std::vector<int> >(field, list) );
        return def;
    }

}
void VSettings::writeFloat(std::string field, float flt)
{
    std::map< std::string, std::vector<int> >::iterator it = settings.find(field);
    std::vector<int> list;
    if (settings.end() == it) {
        settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = settings.find(field);
    }

    (*it).second.clear();
    unsigned char *ptr = (unsigned char *) &flt;
    for (size_t i=0;i<sizeof(float);i++){
        (*it).second.push_back((int)ptr[i]);
    }
}

VSettings *VSettings::getSingleton()
{
    static VSettings vs;
    return &vs;
}

VSettings::~VSettings()
{
    bool first;
    std::string path = settings_path(&first);
    FILE *f = fopenu(path.c_str(),"w");
    for (std::map< std::string, std::vector<int> >::iterator it=settings.begin();
         it!=settings.end();
         it++) {
        fprintf(f,"%s %d ",(*it).first.c_str(),(int)(*it).second.size());
        for (size_t i=0;i<(*it).second.size();i++){
            fprintf(f,"%d ",(*it).second[i]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
}

