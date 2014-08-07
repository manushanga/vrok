
#include <cassert>

#include "network.h"
#include "vputils.h"

std::size_t std::network::network_fwrite(void *buffer, std::size_t size, std::size_t n, void *data)
{
    network_file *nf=(network_file *)data;
    if (nf->filled + size*n < NETWORK_MAX_BUFFER) {
        memcpy(&nf->buffer[nf->filled],buffer,size*n);
        nf->filled+=size*n;
        return size*n;
    } else {
        WARN("Reply too large, contact developer");
        return 0;
    }

}

std::network *std::network::getSingleton()
{
    static std::network net;
    return &net;
}

std::network::network()
{
    curlguard_.lock();
    curl_global_init(CURL_GLOBAL_ALL);
    curl_=curl_easy_init();
    buffer_=new char[NETWORK_MAX_BUFFER];
    curlguard_.unlock();
}

std::network::~network()
{
    curlguard_.lock();
    curl_global_cleanup();
    delete []buffer_;
    curlguard_.unlock();
}

std::network::error_t std::network::network_operation(std::string url, std::network::http_t type, std::network::data_t &data)
{
    curlguard_.lock();
    if (!curl_)
        return NETWORK_NO_NETWORK;

    CURLcode res;
    error_t error=NETWORK_NO_ERROR;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "vrok");

    switch (type)
    {
    case NETWORK_GET:
    {
        network_file nf;
        nf.buffer = buffer_;
        nf.filled = 0;

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, network_fwrite);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &nf);

#ifdef DEBUG
        curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
#endif

        res= curl_easy_perform(curl_);

        nf.buffer[nf.filled] = '\0';

        if (res != CURLE_OK){
            error=NETWORK_NO_CONNECT;
        }
        curl_easy_cleanup(curl_);

    }
    case NETWORK_POST:
    {

    }
    default:
        error=NETWORK_INVALID_INPUT;
    }
    curlguard_.unlock();
    return error;
}
