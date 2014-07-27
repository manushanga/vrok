#ifndef NETWORK_H
#define NETWORK_H

#ifdef VPNETWORK_QT
#include <map>
#include <vector>

#include <QNetworkAccessManager>
#include <QUrl>
#ifndef QT_NO_OPENSSL
namespace std{

class network{
private:

public:
    enum http_t{POST=0,GET};
    enum error_t{NO_CONNECT=0,NO_AUTH};
    typedef map<string, vector<string> > data_t;
    static network *getSingleton();
    error_t network_operation(string url, http_t type, data_t& data);


};
#else
#error "Qt was compiled with no OpenSSL support, Vrok needs secure connections"
#endif
}
#endif
#endif // NETWORK_H
