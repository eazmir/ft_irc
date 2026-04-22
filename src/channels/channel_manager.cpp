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
    
    if (!c.regestred) {
        this->auth.tryRegister(c, input);
        return; 
    }

    std::stringstream ss(input);
    std::string cmd;
    ss >> cmd;

    for (size_t i = 0; i < cmd.size(); i++) 
        cmd[i] = toupper(cmd[i]);

    if (cmd == "JOIN")         handleJoin(input, c);
    else if (cmd == "PART")    handlePart(input, c);
    else if (cmd == "QUIT")    handleQuit(input, c);
    else if (cmd == "PRIVMSG") handlePrivmsg(input, c);
    else if (cmd == "KICK")    handleKick(input, c);
    else if (cmd == "MODE") handleMode(input, c);
    else if (cmd == "TOPIC")   handleTopic(input, c);
    else if (cmd == "INVITE")  handleInvite(input, c);
    else if (cmd == "PRINT")   Utils::printClientsInfo(_clients);
}

Message parseMessage(const std::string &input)
{
    Message msg;

    std::string::size_type pos = input.find(':');

    std::string before;
    if (pos != std::string::npos)
        before = input.substr(0, pos);
    else
        before = input;

    std::string after;
    if (pos != std::string::npos)
        after = input.substr(pos + 1);

    std::istringstream iss(before);
    std::string token;

    bool first = true;
    while (iss >> token)
    {
        if (first)
        {
            msg.command = token;
            first = false;
        }
        else
            msg.args.push_back(token);
    }
    if (!after.empty())
        msg.trailing = after;
    return msg;
}
