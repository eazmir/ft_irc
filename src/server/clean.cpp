#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

void server::clean()
{
    delete this->channel;
    if (_fd_server != -1)
        close(_fd_server);
    for (size_t i = 0; i < _pfds.size();i++)
    {
        if (_pfds[i].fd != -1)
            close(_pfds[i].fd);
    }
    _pfds.clear();
    bye();
}