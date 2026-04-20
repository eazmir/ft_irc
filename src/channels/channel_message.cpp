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

    // 1. Locate the message start (handling the colon correctly)
    size_t pos = input.find(':', input.find(target));
    std::string message;
    if (pos != std::string::npos) {
        message = input.substr(pos); 
    } else {
        // Fallback for non-standard clients
        ss >> message; 
    }

    if (target.empty() || message.empty()) return;

    // 2. The Protocol Shield (Strict 512-byte IRC limit)
    std::string prefix = ":" + c.nickname + " PRIVMSG " + target + " ";
    std::string full_msg = prefix + message;
    
    if (full_msg.size() > 510) {
        full_msg = full_msg.substr(0, 510);
    }
    full_msg += "\r\n";

    // 3. Routing & Security Gates
    if (target[0] == '#') {
        // --- CHANNEL BROADCAST ---
        if (channels.find(target) != channels.end()) {
            Channel *room = channels[target];

            // SECURITY CHECK: Is the sender actually IN this channel?
            bool is_member = false;
            for (size_t i = 0; i < room->members.size(); i++) {
                if (room->members[i] == c.fd) {
                    is_member = true;
                    break;
                }
            }

            if (!is_member) {
                // Error 404: Cannot send to channel (they were likely KICKED or PARTED)
                std::string err = ":ircserv 404 " + c.nickname + " " + target + " :Cannot send to channel\r\n";
                send(c.fd, err.c_str(), err.size(), 0);
                return;
            }

            // Broadcast to everyone EXCEPT the sender
            for (size_t i = 0; i < room->members.size(); i++) {
                if (room->members[i] != c.fd) {
                    send(room->members[i], full_msg.c_str(), full_msg.size(), 0);
                }
            }
        } else {
            // Error 401: No such channel
            std::string err = ":ircserv 401 " + c.nickname + " " + target + " :No such nick/channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    } 
    else {
        // --- PRIVATE MESSAGE TO USER ---
        bool found = false;
        for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->second.nickname == target) {
                send(it->first, full_msg.c_str(), full_msg.size(), 0);
                found = true;
                break;
            }
        }
        
        if (!found) {
            // Error 401: User not found (they likely QUIT)
            std::string err = ":ircserv 401 " + c.nickname + " " + target + " :No such nick/channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    }
}