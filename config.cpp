#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "config.h"
#include "vputils.h"

std::map<std::string, std::vector<int> > __vrok_settings;

#ifdef _WIN32
static std::string settings_path(bool &firsttime)
{

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
    FILE *f=fopen(path.c_str(),"r");
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

static std::string read_string(std::string field)
{
    try{
        std::vector<int> list = __vrok_settings[field];
        char buffer[list.size()+1];
        memset(buffer,'\0',list.size()+1);
        for (size_t i=0;i<list.size();i++){
            buffer[i] = (char)list[i];
        }
        return std::string(buffer);
    } catch (std::exception e) {
        return std::string("");
    }
}
static void write_string(std::string field, std::string str)
{
    std::map< std::string, std::vector<int> >::iterator it = __vrok_settings.find(field);
    std::vector<int> list;
    if (__vrok_settings.end()== it) {
        __vrok_settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = __vrok_settings.find(field);
    }

    (*it).second.clear();
    for (size_t i=0;i<str.length();i++){
        (*it).second.push_back((int)str[i]);
    }
    DBG((*it).second.size());
}
static int read_int(std::string field)
{
    try{
        std::vector<int> list = __vrok_settings[field];
        return list[0];
    } catch (std::exception e) {
        return 0;
    }
}
static void write_int(std::string field, int i)
{
    std::map< std::string, std::vector<int> >::iterator it = __vrok_settings.find(field);
    std::vector<int> list;
    if (__vrok_settings.end() == it) {
        __vrok_settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = __vrok_settings.find(field);
    }

    (*it).second.clear();
    (*it).second.push_back(i);
}
static double read_double(std::string field)
{
    try{
        std::vector<int> list = __vrok_settings[field];
        double d;
        unsigned char *ptr = (unsigned char *)&d;
        for (size_t i=0;i<sizeof(double);i++) {
            ptr[i]=(unsigned char)list[i];
        }
        return d;
    } catch (std::exception e) {
        return 0.0;
    }

}
static void write_double(std::string field, double d)
{
    std::map< std::string, std::vector<int> >::iterator it = __vrok_settings.find(field);
    std::vector<int> list;
    if (__vrok_settings.end() == it) {
        __vrok_settings.insert(std::pair< std::string, std::vector<int> >(field,list));
        it = __vrok_settings.find(field);
    }

    (*it).second.clear();
    unsigned char *ptr = (unsigned char *) &d;
    for (size_t i=0;i<sizeof(double);i++){
        (*it).second.push_back((int)ptr[i]);
    }
}

static void config_write()
{
    bool first;
    std::string path = settings_path(&first);
    FILE *f = fopen(path.c_str(),"w");
    for (std::map< std::string, std::vector<int> >::iterator it=__vrok_settings.begin();
         it!=__vrok_settings.end();
         it++) {
        fprintf(f,"%s %d ",(*it).first.c_str(),(int)(*it).second.size());
        for (size_t i=0;i<(*it).second.size();i++){
            fprintf(f,"%d ",(*it).second[i]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
}

static void config_read()
{
    bool first;
    std::string path = settings_path(&first);
    DBG("config: up");
    if (first) {
        DBG("config: setting defaults");
        write_double("volume",1.0);
        for (int i=0;i<18;i++){
            std::string eqb("eq");
            std::string eqk("eqk");
            write_double(eqb.append(TOSTR(i)),1.0);
            write_double(eqk.append(TOSTR(i)),0.0);
        }
        write_int("eqon",0);
        write_double("eqpre",1.0);
        write_string("lastopen","");
        config_write();
    } else {
        FILE *f = fopen(path.c_str(),"r");

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
            __vrok_settings.insert(std::pair< std::string, std::vector<int> >(std::string(name),list));
        }
        fclose(f);
    }

}


void config_init()
{
    config_read();
}
bool config_get_eq()
{
    return (bool) read_int("eqon");
}
void config_set_eq(bool on)
{
    write_int("eqon",(on)?1:0);
}
std::string config_get_lastopen()
{
    return read_string("lastopen");
}

void config_set_lastopen(std::string last)
{
    write_string("lastopen",last);
}
void config_set_eq_bands(float *bands)
{
    for (int i=0;i<18;i++){
        std::string eqb("eq");
        write_double(eqb.append(TOSTR(i)),(double)bands[i]);
    }
}
void config_set_eq_knowledge_bands(float *bands)
{
    for (int i=0;i<18;i++){
        std::string eqk("eqk");
        write_double(eqk.append(TOSTR(i)),(double)bands[i]);
    }
}
void config_set_eq_preamp(float pa)
{
    write_double("eqpre",(double)pa);
}
float config_get_eq_preamp()
{
    return (float) read_double("eqpre");
}
void config_get_eq_knowledge_bands(float *bands)
{
    for (int i=0;i<18;i++){
        std::string eqk("eqk");
        bands[i] = read_double(eqk.append(TOSTR(i)));
    }
}
void config_get_eq_bands(float *bands)
{
    for (int i=0;i<18;i++){
        std::string eq("eq");
        bands[i] = read_double(eq.append(TOSTR(i)));
    }
}
float config_get_volume()
{
    return (float)read_double("volume");
}
void config_set_volume(float vol)
{
    write_double("volume",(double)vol);
}

void config_finit()
{
    DBG("config: dying");
    config_write();
}
