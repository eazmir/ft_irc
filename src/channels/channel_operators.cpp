/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_operators.cpp                              :+:      :+:    :+:   */
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

void managerchannel::handleKick(const std::string &input, client &c)
{
    std::stringstream ss(input);
    std::string command, channel_name, target_nick, reason;

    ss >> command >> channel_name >> target_nick;

    size_t pos = input.find(target_nick);
    if (pos != std::string::npos) {
        pos += target_nick.length();
        reason = input.substr(pos);
    }
    if (reason.empty() || reason == " ") reason = " :Kicked by operator";

    std::map<std::string, Channel*>::iterator it = channels.find(channel_name);
    if (it == channels.end()) {
        std::string err = ":ircserv 403 " + c.nickname + " " + channel_name + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    Channel *room = it->second;

    bool is_op = false;
    for (size_t i = 0; i < room->operators.size(); i++) {
        if (room->operators[i] == c.fd) {
            is_op = true;
            break;
        }
    }
    if (!is_op) {
        std::string err = ":ircserv 482 " + c.nickname + " " + channel_name + " :You're not channel operator\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    std::vector<int>::iterator vit;
    bool victim_found = false;
    for (vit = room->members.begin(); vit != room->members.end(); ++vit) {
        if (_clients[*vit].nickname == target_nick) {
            int victim_fd = *vit;

            std::string kick_msg = ":" + c.nickname + " KICK " + channel_name + " " + target_nick + " " + reason + "\r\n";

            for (size_t i = 0; i < room->members.size(); i++) {
                send(room->members[i], kick_msg.c_str(), kick_msg.size(), 0);
            }

            room->members.erase(vit);

            for (std::vector<int>::iterator oit = room->operators.begin(); oit != room->operators.end(); ++oit) {
                if (*oit == victim_fd) {
                    room->operators.erase(oit);
                    break;
                }
            }
            
            victim_found = true;
            break; 
        }
    }

    if (!victim_found) {
        std::string err = ":ircserv 441 " + c.nickname + " " + target_nick + " " + channel_name + " :They aren't on that channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
    }
}

void managerchannel::handleTopic(const std::string &input, client &c) 
{
    std::stringstream ss(input);
    std::string command, channel_name;
    ss >> command >> channel_name;

    if (channels.find(channel_name) == channels.end()) {
        std::string err = ":ircserv 403 " + c.nickname + " " + channel_name + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    Channel *room = channels[channel_name];

    bool is_member = false;
    for (size_t i = 0; i < room->members.size(); i++) {
        if (room->members[i] == c.fd) is_member = true;
    }
    if (!is_member) {
        std::string err = ":ircserv 442 " + c.nickname + " " + channel_name + " :You're not on that channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    size_t colon_pos = input.find(':');

    if (colon_pos == std::string::npos) {
        if (room->topic.empty()) {
            std::string msg = ":ircserv 331 " + c.nickname + " " + channel_name + " :No topic is set\r\n";
            send(c.fd, msg.c_str(), msg.size(), 0);
        } else {
            std::string msg = ":ircserv 332 " + c.nickname + " " + channel_name + " :" + room->topic + "\r\n";
            send(c.fd, msg.c_str(), msg.size(), 0);
        }
        return;
    }

    if (room->topic_restricted) {
        bool is_op = false;
        for (size_t i = 0; i < room->operators.size(); i++) {
            if (room->operators[i] == c.fd) is_op = true;
        }
        if (!is_op) {
            std::string err = ":ircserv 482 " + c.nickname + " " + channel_name + " :You're not channel operator\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
            return;
        }
    }

    room->topic = input.substr(colon_pos + 1);
    std::string broadcast = ":" + c.nickname + " TOPIC " + channel_name + " :" + room->topic + "\r\n";
    for (size_t i = 0; i < room->members.size(); i++) {
        send(room->members[i], broadcast.c_str(), broadcast.size(), 0);
    }
}

void managerchannel::handleInvite(const std::string &input, client &c) {
    std::stringstream ss(input);
    std::string command, target_nick, channel_name;
    ss >> command >> target_nick >> channel_name;

    if (target_nick.empty() || channel_name.empty()) {
        std::string err = ":ircserv 461 " + c.nickname + " INVITE :Not enough parameters\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    if (channels.find(channel_name) == channels.end()) {
        std::string err = ":ircserv 403 " + c.nickname + " " + channel_name + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    Channel *room = channels[channel_name];

    bool is_op = false;
    for (size_t i = 0; i < room->operators.size(); i++) {
        if (room->operators[i] == c.fd) is_op = true;
    }
    if (!is_op) {
        std::string err = ":ircserv 482 " + c.nickname + " " + channel_name + " :You're not channel operator\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    for (size_t i = 0; i < room->members.size(); i++) {
        if (_clients[room->members[i]].nickname == target_nick) {
            std::string err = ":ircserv 443 " + c.nickname + " " + target_nick + " " + channel_name + " :is already on channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
            return;
        }
    }

    room->invite_list.push_back(target_nick);

    std::string confirm = ":ircserv 341 " + c.nickname + " " + target_nick + " " + channel_name + "\r\n";
    send(c.fd, confirm.c_str(), confirm.size(), 0);

    int target_fd = -1;

    for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.nickname == target_nick) {
            target_fd = it->first;
            break;
        }
    }

    if (target_fd != -1) {
        std::string invite_msg = ":" + c.nickname + " INVITE " + target_nick + " :" + channel_name + "\r\n";
        send(target_fd, invite_msg.c_str(), invite_msg.size(), 0);
    }
}
