/*
 VPResource is the unification of remote and local media available from
 Vrok 4 onwards. Every path is a URL as defined in the RFC, other than file://
 for file:// the local path is appended as is the way the operating system
 has defined file paths.
*/
#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

class VPResource {
private:
    std::string url_;
public:
    enum VPResourceInitType {INIT_FILE, INIT_URL};

    VPResource(std::string url, VPResourceInitType type);
    std::string getPath();
    std::string getURL();
    std::string getProtocol();
    std::string getExtension();
};
#endif // RESOURCE_H
