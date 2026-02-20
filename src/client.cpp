#include "../include/client.hpp"
#include "../include/server.hpp"
#include <map>
#include <set>

void  managerchannel::print_info()
{
    if (_clients.empty())
    {
        std::cout << "No clients connected" << std::endl;
        return;
    }
    std::map<int, client>::iterator it;
    for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
            std::cout << "FD: " << it->first << std::endl;
            std::cout << "Status: " << it->second.status << std::endl;
            std::cout << "Pass OK: " << it->second.pass_ok << std::endl;
            std::cout << "User OK: " << it->second.user_ok << std::endl;
            std::cout << "Nick OK: " << it->second.nick_ok << std::endl;
            std::cout << "Registered: " << it->second.regestred << std::endl;
            std::cout << "Nickname: " << it->second.nickname << std::endl;
            std::cout << "Username: " << it->second.username << std::endl;
            std::cout << "Real Name: " << it->second.realname << std::endl;
            std::cout << "--------------------------" << std::endl;
    }
}

managerchannel::managerchannel(std::map<int, client> &clients)
    : _clients(clients)
{
}

void managerchannel::handle_input(const std::string &input, client &c)
{
    // For debugging, print the received input and client info
    print_info();
    // Example: Handle PING command
    if (input.compare("PING") == 0)
    {
        std::cout << "Received PING from client FD: " << c.fd << std::endl;
        send(c.fd, "PONG\r\n", 6, 0);
        return;
    }
    if (input.compare("HELLO") == 0)
    {
       std::cout << "Received HELLO from client FD: " << c.fd << std::endl;
       send(c.fd, "Hello client!\r\n", 15, 0);
    }
     // Handle other commands like JOIN, PART, PRIVMSG, etc.
}