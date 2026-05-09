#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

void server::recv_data(size_t &index)
{
    int fd = _pfds[index].fd;

    if (fd == _fd_server)
        return;

    std::map<int, client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;

    char tmp[1024];
    int n = recv(fd, tmp, sizeof(tmp), 0);
    if (n < 0)
    {
        perror("recv");
        return;   
    }
    if (n == 0)
    {
        disconnect_client(index);
        status = true;
        return;
    }
    else if (n > 0)
    {
        it->second.buffer.append(tmp,n);
        std::string line = extract_data(it->second);
        while (!line.empty())
        {
            this->channel->handle_input(line,it->second,auth);
            line = extract_data(it->second);
            if (_clients.find(fd) == _clients.end())
                break;
        }
    }
}