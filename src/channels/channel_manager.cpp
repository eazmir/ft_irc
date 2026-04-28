/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_manager.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 by haitaabe                   #+#    #+#             */
/*   Updated: 2026/04/18 by haitaabe                   ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/channels/channel.hpp"
#include "../../include/utls.hpp"
#include "../../include/server.hpp"

managerchannel::managerchannel(std::map<int, client> &clients,const std::string &pass)
    : _clients(clients),
      auth(pass)
{
}

void managerchannel::handle_input(const std::string &input, client &c)
{
     
    if (input.empty()) return;
    
    std::stringstream ss(input);
    std::string cmd;
    ss >> cmd;

    if (cmd == "/connect")
    {
        this->auth.handlePass(c,input);
    }
    else if (c.first && cmd == "/help")
    {
        Utils::sendHelp(c.fd);
        c.first = false;
    }
    else if (cmd == "/help" && c.regestred)
    {
        Utils::helpchannel(c.fd);
    }
    else if (!c.regestred) 
    {
        if (cmd == "/user" || cmd == "/nick")
            this->auth.tryRegister(c, input);
    }
    for (size_t i = 0; i < cmd.size(); i++) 
        cmd[i] = toupper(cmd[i]);
    if (cmd == "JOIN")         handleJoin(input, c);
    else if (cmd == "PART")    handlePart(input, c);
    else if (cmd == "QUIT")    handleQuit(input, c);
    else if (cmd == "MSG") handlePrivmsg(input, c);
    else if (cmd == "KICK")    handleKick(input, c);
    else if (cmd == "MODE")    handleMode(input, c);
    else if (cmd == "TOPIC")   handleTopic(input, c);
    else if (cmd == "INVITE")  handleInvite(input, c);
}
    // std::cout << "CMD bytes: ";
    // for (std::string::size_type i = 0; i < cmd.size(); i++)
    //     std::cout << std::hex << (int)(unsigned char)cmd[i] << " ";
    // std::cout << std::dec << "\n";