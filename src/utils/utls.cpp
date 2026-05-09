/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utls.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:41:46 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/21 18:46:09 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

std::string Utils::strTrim(const std::string &str) 
{
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == std::string::npos) 
        return ""; 

    size_t end = str.find_last_not_of(" \t\r\n");

    return (str.substr(start, end - start + 1));
}

int Utils::check_port(const std::string& port)
{
    if (port.empty())
        return (-1);
    for (size_t i = 0; i < port.size(); i++) 
    {
        if (!std::isdigit(port[i]))
            return (-1);
    }
    if (port.size() > 5)
        return (-2);
    int _port = std::atoi(port.c_str());
    if (_port < 1 || _port > 65535)
        return (-2);
    if (_port < 1024 || _port > 49151)
        return (-3);
    return (_port);
}

int Utils::validatePort(const std::string& port)
{
    int result = check_port(port);
    if (result == -1)
        throw std::runtime_error("Error : Port must contain digits only.");
    if (result == -2)
        throw std::runtime_error("Error : Port must be between 1 and 65535.");
    if (result == -3)
        throw std::runtime_error("Error : Use a safe port range (1024 - 49151).");
    return result;
}

std::string Utils::getTime()
{
    time_t now = time(0);
    struct tm* tm_info = localtime(&now);
    char buffer[9];
    strftime(buffer, 9, "%H:%M:%S", tm_info);
    return std::string(buffer);
}

std::string Utils::getMachineName()
{
    char hostname[1024];

    if (gethostname(hostname, sizeof(hostname)) == -1)
        return "unknown";

    hostname[sizeof(hostname) - 1] = '\0'; // safety

    return std::string(hostname);
}

void Utils::send_welcome(client &c)
{
    std::string msg; 
    msg = std::string(":ircserv 001 ") + c.nickname +
    " :Welcome to the IRC Network, " +
    c.username + "!\r\n";
    send(c.fd,msg.c_str(),strlen(msg.c_str()),0);
}

void Utils::sendWelcome(int fd)
{
    std::string msg = 
    "(_)_ __ ___ ___  ___ _ ____   __\r\n"
    "| | '__/ __/ __|/ _ \\ '__\\ \\ / /\r\n"
    "| | | | (__\\__ \\  __/ |   \\ V /\r\n" 
    "|_|_|  \\___|___/\\___|_|    \\_/\r\n"
    "::ircserv NOTICE * :Welcome to Our IRC Server\r\n"
    "::ircserv NOTICE * :New to IRC? Type /HELP for assistance\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
}

void Utils::sendRegistrationHelp(int fd)
{
    (void)fd;
    std::stringstream os;

     os << "+_____________________________________________________________________+\r\n";;
    os << "| " << std::setw(68) << std::left
       << "Welcome to Our IRC Server!" << "|\r\n";
    os << "++                                                                   ++\n\r";
    os << "| " << std::setw(68) << std::left
       << "Password  : PASS <password>" << "|\r\n";
    os << "| " << std::setw(68) << std::left
       << "Username  : USER <username> <hostname> <servername>: <realname>" << "|\r\n";
    os << "| " << std::setw(68) << std::left
       << "Nickname  : NICK <nickname>" << "|\r\n";
    os << "| " << std::setw(68) << std::left
       << "\033[4;34mNote: USER <username> 0 * :<realname> (hostname/servername ignored)\033[0m"
<< " |\r\n";
    os << "+_____________________________________________________________________+\r\n";

    std::string msg = os.str();
    send(fd, msg.c_str(), msg.size(), 0);
}

void Utils::helpchannel(int fd)
{
    std::stringstream os;
    os << std::left;

    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(61) << "              Welcome to Our IRC Server!"   << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(61) << "            You are now connected!"         << "|\r\n";
    os << "| " << std::setw(61) << "    To join or chat use the commands below." << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(61) << "                 MESSAGING COMMANDS"         << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(14) << "  join"    << ": " << std::setw(45) << "/join <#channel>"                        << "|\r\n";
    os << "| " << std::setw(14) << "  channel" << ": " << std::setw(45) << "/privmsg <#channel> <message>"           << "|\r\n";
    os << "| " << std::setw(14) << "  private" << ": " << std::setw(45) << "/privmsg <nickname> <message>"           << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(61) << "             CHANNEL OPERATOR COMMANDS"     << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(14) << "  kick"   << ": " << std::setw(45) << "/kick <#channel> <user>"              << "|\r\n";
    os << "| " << std::setw(14) << "  invite" << ": " << std::setw(45) << "/invite <user> <#channel>"            << "|\r\n";
    os << "| " << std::setw(14) << "  topic"  << ": " << std::setw(45) << "/topic <#channel> <topic>"            << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(61) << "                    MODE FLAGS"             << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    os << "| " << std::setw(14) << "  mode i" << ": " << std::setw(45) << "/mode <#channel> +i/-i (invite only)" << "|\r\n";
    os << "| " << std::setw(14) << "  mode t" << ": " << std::setw(45) << "/mode <#channel> +t/-t (topic lock)"  << "|\r\n";
    os << "| " << std::setw(14) << "  mode k" << ": " << std::setw(45) << "/mode <#channel> +k/-k <password>"   << "|\r\n";
    os << "| " << std::setw(14) << "  mode o" << ": " << std::setw(45) << "/mode <#channel> +o/-o <user>"       << "|\r\n";
    os << "| " << std::setw(14) << "  mode l" << ": " << std::setw(45) << "/mode <#channel> +l/-l <limit>"      << "|\r\n";
    os << "+______________________________________________________________+\r\n";
    std::string str = os.str();
    send(fd, str.c_str(), str.size(), 0);
}

std::string Utils::to_str(int n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

