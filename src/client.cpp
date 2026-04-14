/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:41:26 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/14 13:36:11 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/client.hpp"
#include "../include/utls.hpp"

managerchannel::managerchannel(std::map<int, client> &clients,const std::string &pass)
    : _clients(clients),
      auth(pass)
{
}

void managerchannel::handle_input(const std::string &input, client &c)
{
    this->auth.tryRegister(c,input);
    if (input.compare(0,5,"PRINT") == 0)
        Utils::printClientsInfo(_clients);
    if (input.compare(0,4,"JOIN") == 0 || input.compare(0,3,"MSG") == 0)
    {
        handleJoin(input,c);
    }
}

 void managerchannel::handleJoin(const std::string &input, client &c)
 {
    if (!c.regestred)
    {
        std::cout << "You are Not regestred !" << std::endl;
        return;
    }
    else
    {
        std::vector<std::string> local_tokens;
        std::stringstream ss(input);
        std::string token;
        
        while (ss >> token)
        {
            local_tokens.push_back(token);
        }
        if (local_tokens.size() < 2)
        {
           std::cout << "Error: Not enough parameters" << std::endl;
           return;
        }
        this->channel_name = local_tokens[1];
    }
    
    this->it = channels.find(this->channel_name);
    if (it == channels.end())
    {
        Channel *New_ch = new Channel();
        New_ch->name =channel_name;
        channels[this->channel_name] = New_ch;
        this->ch = New_ch;
        
        std::cout << "Success: New channel " << this->channel_name << " created." << std::endl;
    }
    else 
    {
        this->ch = it->second; 
        std::cout << "Joined existing " << this->channel_name << std::endl;
    }
    this->ch->members.push_back(c.fd);
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