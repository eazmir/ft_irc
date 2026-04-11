#include "../include/utls.hpp"
#include "../include/server.hpp"

std::string Utils::strTrim(const std::string &str) 
{
    size_t start = 0;
    size_t end = str.size();
    
    if (str[start] != ' ' || str[start] != '\t')
        return ""; 
    // Trim start
    while (start < end && (str[start] == ' ' || str[start] == '\t'))
        start++;
    // Trim end
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'))
        end--;
    return (str.substr(start, end - start));
}


void Utils::printClientsInfo(const std::map<int, client> &clients)
{
    if (clients.empty())
    {
        std::cout << "No clients connected" << std::endl;
        return;
    }
    std::cout << std::left;
    std::cout << "----------------------------------------------------------"
              << std::endl;
    std::cout << "| " << std::setw(4)  << "FD"
              << "| " << std::setw(5)  << "Pass"
              << "| " << std::setw(5)  << "User"
              << "| " << std::setw(5)  << "Nick"
              << "| " << std::setw(4)  << "Reg"
              << "| " << std::setw(10) << "Nickname"
              << "| " << std::setw(10) << "Username"
              << "|" << std::endl;

    std::cout << "----------------------------------------------------------"
              << std::endl;
    for (std::map<int, client>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        const client &c = it->second;

        std::cout << "| " << std::setw(4)  << it->first
                  << "| " << std::setw(5)  << c.pass_ok
                  << "| " << std::setw(5)  << c.user_ok
                  << "| " << std::setw(5)  << c.nick_ok
                  << "| " << std::setw(4)  << c.regestred
                  << "| " << std::setw(10) << c.nickname
                  << "| " << std::setw(10) << c.username
                  << "|" << std::endl;
    }
}