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

static void removeClientFromChannel(Channel *room, int fd)
{
    for (std::vector<int>::iterator it = room->members.begin(); it != room->members.end();)
    {
        if (*it == fd)
            it = room->members.erase(it);
        else
            ++it;
    }

    for (std::vector<int>::iterator it = room->operators.begin(); it != room->operators.end();)
    {
        if (*it == fd)
            it = room->operators.erase(it);
        else
            ++it;
    }
}



std::vector<std::string> managerchannel::splitByComma(const std::string &s) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty())
            parts.push_back(item);
    }
    return parts;
}

void managerchannel::handleJoin(const std::string &input, client &c)
{
    if (!c.regestred) {
        std::string err = ":ircserv 451 " + (c.nickname.empty() ? "*" : c.nickname) + " :You have not registered\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string t;
    while (ss >> t)
        tokens.push_back(t);

    if (tokens.size() < 2) {
        std::string err = ":ircserv 461 " + c.nickname + " JOIN :Not enough parameters\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    std::vector<std::string> chan_list = splitByComma(tokens[1]);
    std::vector<std::string> key_list;
    if (tokens.size() >= 3)
        key_list = splitByComma(tokens[2]);

    for (size_t i = 0; i < chan_list.size(); i++)
    {
        if (chan_list[i].empty() || chan_list[i][0] != '#')
        {
            std::string err = ":ircserv 476 " + c.nickname + " " + chan_list[i] + " :Bad Channel Mask\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
            return;
        }
    }

    for (size_t i = 0; i < chan_list.size(); i++) 
    {
        std::string current_name = chan_list[i];
        std::string current_key = (i < key_list.size()) ? key_list[i] : "";

        Channel *targetChannel = NULL;
        std::map<std::string, Channel*>::iterator it_chan = channels.find(current_name);

        if (it_chan == channels.end()) {
            targetChannel = new Channel();
            targetChannel->name = current_name;
            targetChannel->limit = 0;
            targetChannel->invite_only = false;
            targetChannel->topic_restricted = false;
            targetChannel->password = "";
            
            channels[current_name] = targetChannel;
            targetChannel->members.push_back(c.fd);
            targetChannel->operators.push_back(c.fd);
            std::cout << "[" << Utils::getTime() << "] "
            << "User " <<c.username+"!"+c.nickname+"@"+c.hostname
            << " join "+ current_name + " (fd=" << c.fd
            << ", ip=" << c.ip << ":" << c.port << ")"
            << std::endl;

        }
        else {
            std::cout << "[" << Utils::getTime() << "] "
            << "User " <<c.username+"!"+c.nickname+"@"+c.hostname
            << " join "+ current_name + " (fd=" << c.fd
            << ", ip=" << c.ip << ":" << c.port << ")"
            << std::endl;
            targetChannel = it_chan->second;

            if (targetChannel->invite_only) {
                bool invited = false;
                for (size_t j = 0; j < targetChannel->invite_list.size(); j++) {
                    if (targetChannel->invite_list[j] == c.nickname) {
                        invited = true;
                        targetChannel->invite_list.erase(targetChannel->invite_list.begin() + j);
                        break;
                    }
                }
                if (!invited) {
                    std::string err = ":ircserv 473 " + c.nickname + " " + current_name + " :Cannot join channel (+i)\r\n";
                    send(c.fd, err.c_str(), err.size(), 0);
                    continue;
                }
            }

            if (targetChannel->limit > 0 && targetChannel->members.size() >= (size_t)targetChannel->limit) {
                std::string err = ":ircserv 471 " + c.nickname + " " + current_name + " :Cannot join channel (+l)\r\n";
                send(c.fd, err.c_str(), err.size(), 0);
                continue;
            }

            if (!targetChannel->password.empty()) {
                if (current_key != targetChannel->password) {
                    std::string err = ":ircserv 475 " + c.nickname + " " + current_name + " :Cannot join channel (+k)\r\n";
                    send(c.fd, err.c_str(), err.size(), 0);
                    continue;
                }
            }

            bool already_member = false;
            for (size_t m = 0; m < targetChannel->members.size(); m++) {
                if (targetChannel->members[m] == c.fd) {
                    already_member = true;
                    break;
                }
            }
            if (!already_member)
                targetChannel->members.push_back(c.fd);
            else
                continue;
        }

        std::string join_msg = ":" + c.nickname + "!" + c.username + "@localhost JOIN " + current_name + "\r\n";
        for (size_t m = 0; m < targetChannel->members.size(); m++) {
            send(targetChannel->members[m], join_msg.c_str(), join_msg.size(), 0);
        }

        if (!targetChannel->topic.empty()) {
            std::string rpl_topic = ":ircserv 332 " + c.nickname + " " + current_name + " :" + targetChannel->topic + "\r\n";
            send(c.fd, rpl_topic.c_str(), rpl_topic.size(), 0);
        }

        std::string names_list = ":ircserv 353 " + c.nickname + " = " + current_name + " :";
        for (size_t m = 0; m < targetChannel->members.size(); m++) {
            bool is_op = false;
            for (size_t o = 0; o < targetChannel->operators.size(); o++) {
                if (targetChannel->operators[o] == targetChannel->members[m]) is_op = true;
            }
            
            int member_fd = targetChannel->members[m];
            std::string m_nick = _clients[member_fd].nickname;
            
            if (is_op) names_list += "@";
            names_list += m_nick + " ";
        }
        names_list += "\r\n";
        send(c.fd, names_list.c_str(), names_list.size(), 0);

        std::string end_names = ":ircserv 366 " + c.nickname + " " + current_name + " :End of /NAMES list.\r\n";
        send(c.fd, end_names.c_str(), end_names.size(), 0);
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

                removeClientFromChannel(room, c.fd);
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

        for (std::vector<int>::iterator vit = room->members.begin(); vit != room->members.end(); ++vit)
        {
            if (*vit == c.fd)
            {
                for (size_t i = 0; i < room->members.size(); i++)
                {
                    if (room->members[i] != c.fd)
                        send(room->members[i], quit_msg.c_str(), quit_msg.size(), 0);
                }
                removeClientFromChannel(room, c.fd);
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
    std::cout << "[" << Utils::getTime() << "] "
          << "[QUIT] User " << c.nickname
          << " left the server (fd=" << c.fd
          << ", ip=" << c.ip << ":" << c.port << ")"
          << std::endl;
}