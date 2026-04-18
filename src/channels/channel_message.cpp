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

    // 1. Locate the message start
    size_t pos = input.find(':', input.find(target));
    std::string message;
    if (pos != std::string::npos) {
        message = input.substr(pos); 
    } else {
        // Fallback if the user forgot the colon
        ss >> message; 
    }

    if (target.empty() || message.empty()) return;

    // 2. The Protocol Shield (512-byte limit)
    // We format the message and ensure it doesn't exceed 510 chars (+ \r\n)
    std::string prefix = ":" + c.nickname + " PRIVMSG " + target + " ";
    std::string full_msg = prefix + message;
    
    if (full_msg.size() > 510) {
        full_msg = full_msg.substr(0, 510);
    }
    full_msg += "\r\n";

    // 3. Routing
    if (target[0] == '#') {
        if (channels.find(target) != channels.end()) {
            Channel *room = channels[target];
            // Broadcast to everyone else in the channel
            for (size_t i = 0; i < room->members.size(); i++) {
                if (room->members[i] != c.fd) {
                    send(room->members[i], full_msg.c_str(), full_msg.size(), 0);
                }
            }
        }
    } 
    else {
        // Private Message to a user
        for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->second.nickname == target) {
                send(it->first, full_msg.c_str(), full_msg.size(), 0);
                break;
            }
        }
    }
}
