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
    this->auth.tryRegister(c,input);
    if (input.compare(0, 5, "PRINT") == 0)
        Utils::printClientsInfo(_clients);

    else if (input.compare(0, 5, "JOIN ") == 0 || input == "JOIN") 
    {
        handleJoin(input, c);
    }
    else if (input.compare(0, 5, "PART ") == 0 || input == "PART")
    {
        handlePart(input, c);
    }
    else if (input.compare(0, 5, "QUIT ") == 0 || input == "QUIT")
    {
        handleQuit(input, c);
    }

    else if (input.compare(0, 8, "PRIVMSG ") == 0) 
    {
        handlePrivmsg(input, c);
    }

    else if (input.compare(0, 5, "KICK ") == 0 || input == "KICK") 
    {
        handleKick(input, c);
    }
    else if (input.compare(0, 6, "TOPIC ") == 0 || input == "TOPIC") 
    {
        handleTopic(input, c);
    }
    else if (input.compare(0, 7, "INVITE ") == 0 || input == "INVITE")
    {
        handleInvite(input, c);
    }
    else if (input.compare(0, 5, "MODE ") == 0 || input == "MODE") 
    {
        handleMode(input, c);
    }
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
