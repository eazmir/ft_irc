#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

std::string server::extract_data(client &c)
{
    std::string &buf = c.buffer;

    size_t pos = buf.find("\r\n");
    if (pos == std::string::npos)
        return "";

    std::string line = buf.substr(0, pos);
    buf.erase(0, pos + 2);
    return (line);
}