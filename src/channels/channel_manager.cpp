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

managerchannel::~managerchannel()
{
   for (it = channels.begin(); it != channels.end();it++)
   {
        delete it->second;
   }
   channels.clear();
}

managerchannel::managerchannel(std::map<int, client> &clients,const std::string &pass)
    : _clients(clients)
{
    (void)pass;
}

void managerchannel::handle_input(const std::string &input, client &c ,authentication &auth)
{
     if (input.empty()) 
        return;
    
    std::stringstream ss(input);
    std::string cmd;

    ss >> cmd;
    if (c.first && cmd == "/HELP")
    {
        Utils::sendRegistrationHelp(c.fd);
        c.first = false;
    }
    else if (cmd == "/HELP" && c.regestred)
    {
        Utils::helpchannel(c.fd);
    }
    if (!c.regestred)
    {
        if (cmd == "USER" || cmd == "NICK" || cmd == "PASS")
            auth.tryRegister(c, input);
    }
    for (size_t i = 0; i < cmd.size(); i++) 
        cmd[i] = toupper(cmd[i]);
    if (cmd == "JOIN")         handleJoin(input, c);
    else if (cmd == "PART")    handlePart(input, c);
    else if (cmd == "QUIT")    handleQuit(input, c);
    else if (cmd == "PRIVMSG") handlePrivmsg(input, c);
    else if (cmd == "KICK")    handleKick(input, c);
    else if (cmd == "MODE")    handleMode(input, c);
    else if (cmd == "TOPIC")   handleTopic(input, c);
    else if (cmd == "INVITE")  handleInvite(input, c);
}