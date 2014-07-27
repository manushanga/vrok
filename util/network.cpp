#include "network.h"

std::network *std::network::getSingleton()
{
    static std::network net;
    return &net;
}

std::network::error_t std::network::network_operation(std::string url, std::network::http_t type, std::network::data_t &data)
{

}
