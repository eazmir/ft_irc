#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

void server::handleEvent()
{
    signalhandler();
    while (g_status)
    {
        poll(_pfds.data(),_pfds.size(),-1);
        for (size_t client_index = 0; client_index < _pfds.size();client_index++)
        {
            if (_pfds[client_index].revents & POLLIN)
            {
                if (_fd_server == _pfds[client_index].fd)
                {
                    accept_connection();
                    client_index++;
                }
                else
                {
                    recv_data(client_index);
                    if (status)
                    {
                        status = false;
                        client_index--;
                    }
                }   
            }
        }
    }
}