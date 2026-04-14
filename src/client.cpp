/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:41:26 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/14 19:45:19 by haitaabe         ###   ########.fr       */
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

void test(std::vector<int> members)
{
    for (size_t i = 0; i < members.size() ; i++)
    {
        std::cout<<" "<<members[i]<<std::endl;
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
        New_ch->name = channel_name;
        New_ch->limit = 0;
        New_ch->invite_only = false;
        New_ch->topic_restricted = false;
        channels[this->channel_name] = New_ch;
        this->ch = New_ch;
        this->ch->members.push_back(c.fd);
        this->ch->operators.push_back(c.fd); // ✅ First joiner becomes operator
       
        std::cout << "Success: New channel " << this->channel_name << " created." << std::endl;
        // std::string msg = "Success: New channel " + this->channel_name + " created.\r\n";
        // send(c.fd, msg.c_str(), msg.size(), 0);
    }
    else 
    {
        this->ch = it->second; 
        std::cout<<"__________________________________________\n";
        this->ch->members.push_back(c.fd);
        std::cout<<"members: \n";
        test(this->ch->members);
        std::cout<<"__________________________________________\n";
        std::cout<<"admin: ";
        test(this->ch->operators);
        std::cout<<"__________________________________________\n";
    }


    std::string prefix = ":" + c.nickname + "!" + c.username + "@localhost";
    std::string join_msg = prefix + " JOIN " + this->channel_name + "\r\n";
    for (size_t i = 0; i < this->ch->members.size(); i++)
    {
        int target_fd = this->ch->members[i];
        send(target_fd, join_msg.c_str(), join_msg.size(), 0);
    }
 }

//  Message parseMessage(const std::string &input)
// {
//     Message msg;

//     std::string::size_type pos = input.find(':');

//     std::string before;
//     if (pos != std::string::npos)
//         before = input.substr(0, pos);
//     else
//         before = input;

//     std::string after;
//     if (pos != std::string::npos)
//         after = input.substr(pos + 1);

//     std::istringstream iss(before);
//     std::string token;

//     bool first = true;
//     while (iss >> token)
//     {
//         if (first)
//         {
//             msg.command = token;
//             first = false;
//         }
//         else
//             msg.args.push_back(token);
//     }
//     if (!after.empty())
//         msg.trailing = after;
//     return msg;
// }