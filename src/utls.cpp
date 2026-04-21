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

#include "../include/utls.hpp"
#include "../include/server.hpp"

std::string Utils::strTrim(const std::string &str) 
{
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == std::string::npos) 
        return ""; 

    size_t end = str.find_last_not_of(" \t\r\n");

    return (str.substr(start, end - start + 1));
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