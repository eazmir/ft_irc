/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_message.cpp                                :+:      :+:    :+:   */
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

void managerchannel::handlePrivmsg(const std::string &input, client &c) {
    std::stringstream ss(input);
    std::string command, target;
    ss >> command >> target;

    if (target.empty()) return;

    size_t pos = input.find(':', input.find(target));
    std::string message;

    if (pos != std::string::npos) {
        message = input.substr(pos + 1); 
    } 
    else {
        size_t target_pos = input.find(target);
        message = input.substr(target_pos + target.length());
        size_t first_char = message.find_first_not_of(" \t");
        if (first_char != std::string::npos)
            message = message.substr(first_char);
    }

    if (message.empty()) return;

    // Formatting for the IRC client
    std::string full_msg = ":" + c.nickname + " PRIVMSG " + target + " :" + message;
    if (full_msg.size() > 510) full_msg = full_msg.substr(0, 510);
    full_msg += "\r\n";

    if (target[0] == '#') {
        // CHANNEL LOGIC
        if (channels.find(target) != channels.end()) {
            Channel *room = channels[target];
            bool is_member = false;
            for (size_t i = 0; i < room->members.size(); i++) {
                if (room->members[i] == c.fd) { is_member = true; break; }
            }

            if (!is_member) {
                std::string err = ":ircserv 404 " + c.nickname + " " + target + " :Cannot send to channel\r\n";
                send(c.fd, err.c_str(), err.size(), 0);
                return;
            }

            for (size_t i = 0; i < room->members.size(); i++) {
                if (room->members[i] != c.fd) {
                    send(room->members[i], full_msg.c_str(), full_msg.size(), 0);
                }
            }
        } else {
            std::string err = ":ircserv 401 " + c.nickname + " " + target + " :No such nick/channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    } 
    else {
        // PRIVATE MESSAGE LOGIC
        bool found = false;
        
        // Convert target to uppercase for case-insensitive search
        std::string targetUpper = target;
        for (size_t i = 0; i < targetUpper.size(); i++) targetUpper[i] = toupper(targetUpper[i]);

        for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            std::string currentNickUpper = it->second.nickname;
            for (size_t i = 0; i < currentNickUpper.size(); i++) currentNickUpper[i] = toupper(currentNickUpper[i]);

            if (currentNickUpper == targetUpper) {
                send(it->first, full_msg.c_str(), full_msg.size(), 0);
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "[DEBUG] Private target not found: " << target << std::endl;
            std::string err = ":ircserv 401 " + c.nickname + " " + target + " :No such nick/channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    }
}