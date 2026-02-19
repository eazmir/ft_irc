#include "../include/client.hpp"
#include "../include/server.hpp"

managerchannel::managerchannel(std::vector<client> &clients)
    : _clients(clients)
{
}

void managerchannel::handle_input(const std::string &input, client &c)
{
    if (input.compare(0, 7, "PRIVMSG") == 0)
    {
        std::string message = input.substr(8); // skip "PRIVMSG "
        for (size_t i = 0; i < _clients.size(); i++)
        {
            client &other_client = _clients[i];
            if (other_client.fd != c.fd)
            {
                send(other_client.fd, message.c_str(), message.size(), 0);
            }
        }
    }
    // for exmaple
    else if (input.compare(0,4,"JOIN") == 0)
    {
        //........................
    }
    else if (input.compare(0,4,"NICK"))
    {
        //-------------------------
    }

}