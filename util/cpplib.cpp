#include "cpplib.h"

namespace std{
template<>
/** Convert to int
 */
int from_string<int>(std::string number, int base)
{
    int power=1;
    int i=number.length()-1,final=0;
    while (i>=0)
    {
        final+=power*cpplib::char_to_int(number.c_str()[i]);
        power*=base;
        i--;
    }
    return final;
}
template<>
/** Convert to float
 */
float from_string<float>(std::string number, float base)
{

    float final=0.0f;
    int radix=number.find('.');
    int i=number.length()-1;

    if (radix!= std::string::npos){
        float power=pow(base,-1.0f*(number.length() -radix -1));
        while (i>=0)
        {
            if (number.c_str()[i] != '.'){

                final+=power*static_cast<float>(cpplib::char_to_int(number.c_str()[i]));
                power*=base;
            }
            i--;
        }
    } else {
        float power=1.0f;
        while (i>=0)
        {
            final+=power*static_cast<float>(cpplib::char_to_int(number.c_str()[i]));
            power*=base;
            i--;
        }
    }
    return final;
}

template<>
/** Convert to double
 */
double from_string<double>(std::string number, double base)
{

    double final=0.0;
    int radix=number.find('.');
    int i=number.length()-1;

    if (radix!= std::string::npos){
        double power=pow(base,-1.0*(number.length() -radix -1));
        while (i>=0)
        {
            if (number.c_str()[i] != '.'){

                final+=power*static_cast<double>(cpplib::char_to_int(number.c_str()[i]));
                power*=base;
            }
            i--;
        }
    } else {
        double power=1.0;
        while (i>=0)
        {
            final+=power*static_cast<double>(cpplib::char_to_int(number.c_str()[i]));
            power*=base;
            i--;
        }
    }
    return final;
}
std::string to_upper(std::string str)
{
    char *sptr=(char*)str.c_str();
    std::string final;
    final.resize(str.size());
    char *tptr=(char*)final.c_str();

    while (*sptr!='\0'){
        *tptr=toupper(*sptr);
        tptr++;
        sptr++;
    }
    tptr='\0';
    return final;
}

std::string to_lower(std::string str)
{
    char *sptr=(char*)str.c_str();
    std::string final;
    final.resize(str.size());
    char *tptr=(char*)final.c_str();

    while (*sptr!='\0'){
        *tptr=tolower(*sptr);
        tptr++;
        sptr++;
    }

    tptr='\0';
    return final;
}

std::string trim(std::string str)
{
    char *sptr=(char*)str.c_str(),*eptr=(char*)str.c_str() + str.length() -1;
    while (isspace(*sptr)) {
        sptr++;
    }
    while (isspace(*eptr)) {
        eptr--;
    }
    *eptr='\0';
    return std::string(sptr);
}

void split(std::string str, char delim, std::vector<std::string>& strvec)
{
    char *sptr=(char*)str.c_str(),*eptr=(char*)str.c_str();
    while (*eptr!='\0'){
        if (*eptr == delim){
            char tmp = *eptr;
            *eptr='\0';
            if (*sptr != '\0')
                strvec.push_back( std::string(sptr) );
            *eptr=tmp;
            sptr=eptr+1;
        }
        eptr++;
    }
    if (*sptr != '\0')
        strvec.push_back( std::string(sptr) );
}

void split(std::string str, char *delim, std::vector<std::string>& strvec)
{
    char *sptr=(char*)str.c_str(),*eptr=(char*)str.c_str();
    char *delptr;
    while (*eptr!='\0'){
        delptr=delim;

        while (*delptr!='\0') {
            if (*delptr == *eptr)
            {
                char tmp = *eptr;
                *eptr='\0';
                if (*sptr != '\0')
                    strvec.push_back( std::string(sptr) );
                *eptr=tmp;
                sptr=eptr+1;
                break;
            }
            delptr++;
        }

        eptr++;
    }
    if (*sptr != '\0')
        strvec.push_back( std::string(sptr) );
}

}
