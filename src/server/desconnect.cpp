#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

void server::disconnect_client(size_t &index)
{
    int fd = _pfds[index].fd;

    std::map<int, client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        channel->handleQuit("QUIT :Remote host closed the connection", it->second);
    }
    close(fd);
    _clients.erase(fd);
    _pfds.erase(_pfds.begin() + index);
}
