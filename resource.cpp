#include "resource.h"
#include <iostream>

/**
 *
 *
 */
VPResource::VPResource(std::string url, VPResourceInitType type)
{
    if (type == INIT_FILE)
        url_ = "file://" + url;
    else
        url_ = url;
}

std::string VPResource::getPath()
{
    return url_.substr( url_.find_first_of(':') +3);
}

std::string VPResource::getURL()
{
    return url_;
}

std::string VPResource::getProtocol()
{
    return url_.substr( 0, url_.find_first_of(':') );
}
std::string VPResource::getExtension()
{
    return url_.substr( url_.find_last_of('.') +1 );
}
