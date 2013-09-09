#ifndef CPPUTILS_H
#define CPPUTILS_H

#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <cmath>
#include <cctype>
// Madura A.
// C++ Utilities
// helpers
namespace cpplib {
inline int char_to_int(char c)
{
    switch (c){
    case 'A':
        return 10;
    case 'B':
        return 11;
    case 'C':
        return 12;
    case 'D':
        return 13;
    case 'E':
        return 14;
    case 'F':
        return 15;
    default:
        return c - '0';
    }
}
}
namespace std{
template<typename FROM>
/** Convert to wstring from FROM type
 */
std::wstring to_wstring(FROM f){
    std::wstringstream ss;
    ss<<f;
    return ss.str();
}
template<typename FROM>
/** Convert to string from FROM type
 */
std::string to_string(FROM f){
    std::stringstream ss;
    ss<<f;
    return ss.str();
}
/** Get upper case string
 */
std::string to_upper(std::string str);
/** Get lower case string
 */
std::string to_lower(std::string str);
/** Trim string from start and end.
	@remarks
	Uses isspace() to check for blanks
 */
std::string trim(std::string str);
/**
	Splits a string by delim (char) and puts in the given vector
  */
void split(std::string str, char delim, std::vector<std::string>& strvec);
/**
	Splits a string by delim (c_str) and puts in the given vector
  */
void split(std::string str, char *delim, std::vector<std::string>& strvec);

template<typename TO>
/** TO=conversion type, 0 < base <= 16
 */
TO from_string(std::string number, TO base)
{
    return (TO)0;
}


}

#endif // CPPUTILS_H
