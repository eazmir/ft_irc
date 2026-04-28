/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_join_part.cpp                              :+:      :+:    :+:   */
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



void managerchannel::handleJoin(const std::string &input, client &c)
{
    if (!c.regestred) {
        std::cout << "Error: Client not registered" << std::endl;
        return;
    }

    std::vector<std::string> local_tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token)
        local_tokens.push_back(token);

    if (local_tokens.size() < 2) {
        std::string err = ":ircserv 461 " + c.nickname + " JOIN :Not enough parameters\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    
    this->channel_name = local_tokens[1];
    this->it = channels.find(this->channel_name);

    if (it == channels.end()) {
        Channel *New_ch = new Channel();
        New_ch->name = this->channel_name;
        New_ch->limit = 0;
        New_ch->invite_only = false;
        New_ch->topic_restricted = false;
        
        channels[this->channel_name] = New_ch;
        this->ch = New_ch;
        this->ch->members.push_back(c.fd);
        this->ch->operators.push_back(c.fd);
    }
    else {
        this->ch = it->second;

        if (this->ch->invite_only) {
            bool invited = false;
            for (size_t i = 0; i < this->ch->invite_list.size(); i++) {
                if (this->ch->invite_list[i] == c.nickname) {
                    invited = true;
                    this->ch->invite_list.erase(this->ch->invite_list.begin() + i);
                    break;
                }
            }
            if (!invited) {
                std::string err = ":ircserv 473 " + c.nickname + " " + this->channel_name + " :Cannot join channel (+i)\r\n";
                send(c.fd, err.c_str(), err.size(), 0);
                return;
            }
        }

        if (this->ch->limit > 0 && this->ch->members.size() >= (size_t)this->ch->limit) {
            std::string err = ":ircserv 471 " + c.nickname + " " + this->channel_name + " :Cannot join channel (+l)\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
            return;
        }

        if (!this->ch->password.empty()) {
            if (local_tokens.size() < 3 || local_tokens[2] != this->ch->password) {
                std::string err = ":ircserv 475 " + c.nickname + " " + this->channel_name + " :Cannot join channel (+k)\r\n";
                send(c.fd, err.c_str(), err.size(), 0);
                return;
            }
        }

        bool already_member = false;
        for (size_t i = 0; i < this->ch->members.size(); i++) {
            if (this->ch->members[i] == c.fd) already_member = true;
        }
        if (!already_member)
            this->ch->members.push_back(c.fd);
    }

    std::string prefix = ":" + c.nickname + "!" + c.username + "@localhost";
    std::string join_msg = prefix + " JOIN " + this->channel_name + "\r\n";
    for (size_t i = 0; i < this->ch->members.size(); i++) {
        send(this->ch->members[i], join_msg.c_str(), join_msg.size(), 0);
    }
}

void managerchannel::handlePart(const std::string &input, client &c)
{
    std::stringstream scanner(input);
    std::string command, target, reason;

    if (!(scanner >> command >> target))
        return;

    while (!target.empty() && (target[target.size() - 1] == '\r' || target[target.size() - 1] == '\n'))
    {
        target.erase(target.size() - 1);
    }

    size_t pos = input.find(':');
    if (pos != std::string::npos) {
        reason = input.substr(pos);
        while (!reason.empty() && (reason[reason.size() - 1] == '\r' || reason[reason.size() - 1] == '\n'))
        {
            reason.erase(reason.size() - 1);
        }
    } else {
        reason = ":Leaving";
    }

    std::map<std::string, Channel*>::iterator it = channels.find(target);
    if (it != channels.end())
    {
        Channel* room = it->second;
        bool found = false;

        for (std::vector<int>::iterator vit = room->members.begin(); vit != room->members.end(); ++vit)
        {
            if (*vit == c.fd)
            {
                std::string part_msg = ":" + c.nickname + "!" + c.username + "@localhost PART " + target + " " + reason + "\r\n";
                
                for (size_t i = 0; i < room->members.size(); i++)
                {
                    send(room->members[i], part_msg.c_str(), part_msg.size(), 0);
                }

                room->members.erase(vit);
                if (room->members.empty()) 
                {
                    delete room; 
                    channels.erase(it); // Remove it from the map too
                }
                found = true;
                break; 
            }
        }
        if (!found) {
            std::string err = ":ircserv 442 " + c.nickname + " " + target + " :You're not on that channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    }
    else {
        std::string err = ":ircserv 403 " + c.nickname + " " + target + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
    }
}

void managerchannel::handleQuit(const std::string &input, client &c)
{
    std::stringstream scanner(input);
    std::string command, reason;
    
    scanner >> command;

    size_t pos = input.find(':');
    if (pos != std::string::npos)
        reason = input.substr(pos);
    else
        reason = ":Client Quit";

    std::string quit_msg = ":" + c.nickname + "!" + c.username + "@localhost QUIT " + reason + "\r\n";

    std::map<std::string, Channel*>::iterator it = channels.begin();
    while (it != channels.end())
    {
        Channel* room = it->second;
        // REMOVED: bool user_found_in_room = false;

        for (std::vector<int>::iterator vit = room->members.begin(); vit != room->members.end(); ++vit)
        {
            if (*vit == c.fd)
            {
                for (size_t i = 0; i < room->members.size(); i++)
                {
                    if (room->members[i] != c.fd)
                        send(room->members[i], quit_msg.c_str(), quit_msg.size(), 0);
                }
                room->members.erase(vit);
                // REMOVED: user_found_in_room = true;
                break; 
            }
        }

        if (room->members.empty()) 
        {
            delete room; 
            std::map<std::string, Channel*>::iterator to_erase = it;
            ++it;
            channels.erase(to_erase); 
        }
        else 
        {
            ++it;
        }
    }
    std::cout <<"["<<Utils::getTime()<<"] "<< "[quit] User " << c.nickname << " has left the server." << std::endl;
}