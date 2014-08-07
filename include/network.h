#ifndef NETWORK_H
#define NETWORK_H

#ifdef VPNETWORK_QT
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <memory>

#include <curl/curl.h>

#include "threads.h"

#define NETWORK_MAX_BUFFER 1024

namespace std{

class network{
private:
    mutex curlguard_;
    CURL *curl_;
    char *buffer_;

    static size_t network_fwrite(void *buffer, size_t size, size_t n, void *data);

public:
    enum http_t{NETWORK_POST=0,NETWORK_GET};
    enum error_t{NETWORK_NO_ERROR=0,
                 NETWORK_NO_NETWORK,
                 NETWORK_NO_CONNECT,
                 NETWORK_NO_AUTH,
                 NETWORK_INVALID_INPUT};
    typedef map<string, vector<string> > data_t;
    struct network_file{
        char *buffer;
        size_t filled;
    };
    static network *getSingleton();
    network();
    ~network();
    error_t network_operation(string url, http_t type, data_t& data);


};
}
#endif
#endif // NETWORK_H
