/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel_modes.cpp                                  :+:      :+:    :+:   */
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

void managerchannel::handleMode(const std::string &input, client &c) {
    std::stringstream ss(input);
    std::string command, target, modeString, param;
    ss >> command >> target >> modeString;

    if (target.empty() || target[0] != '#') return; 
    
    if (channels.find(target) == channels.end()) {
        std::string err = ":ircserv 403 " + c.nickname + " " + target + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    Channel *room = channels[target];

    bool is_op = false;
    for (size_t i = 0; i < room->operators.size(); i++) {
        if (room->operators[i] == c.fd) {
            is_op = true;
            break;
        }
    }

    if (!is_op) {
        std::string err = ":ircserv 482 " + c.nickname + " " + target + " :You're not channel operator\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

    bool adding = true; 
    std::string appliedModes = "";
    
    for (size_t i = 0; i < modeString.length(); i++) 
    {
        char mode = modeString[i];
        if (mode == '+') { adding = true; appliedModes += "+"; continue; }
        if (mode == '-') { adding = false; appliedModes += "-"; continue; }

        if (mode == 'i') {
            room->invite_only = adding;
            appliedModes += "i";
        } 
        else if (mode == 't') {
            room->topic_restricted = adding;
            appliedModes += "t";
        }
        else if (mode == 'k') {
            if (adding) {
                if (ss >> param) { 
                    room->password = param;
                    appliedModes += "k " + param + " ";
                }
            } else {
                room->password = "";
                appliedModes += "k";
            }
        }
        else if (mode == 'l') {
            if (adding) {
                if (ss >> param) {
                    room->limit = std::atoi(param.c_str());
                    appliedModes += "l " + param + " ";
                }
            } else {
                room->limit = 0;
                appliedModes += "l";
            }
        }
        // Inside the mode loop for 'o'
        else if (mode == 'o') {
            if (ss >> param) {
                int target_fd = -1;
                for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
                    if (it->second.nickname == param) {
                        target_fd = it->first;
                        break;
                    }
                }
                if (target_fd != -1) {
                    if (adding) {
                        bool already_op = false;
                        for(size_t j=0; j < room->operators.size(); j++) 
                            if(room->operators[j] == target_fd) already_op = true;
                        if (!already_op) room->operators.push_back(target_fd);
                    } else {
                        for (std::vector<int>::iterator oit = room->operators.begin(); oit != room->operators.end(); ++oit) {
                            if (*oit == target_fd) { room->operators.erase(oit); break; }
                        }
                    }
                    appliedModes += (adding ? "+o " : "-o ") + param + " ";
                }
            }
        }
        
    }

    if (!appliedModes.empty()) {
        std::string mode_msg = ":" + c.nickname + " MODE " + target + " " + appliedModes + "\r\n";
        for (size_t i = 0; i < room->members.size(); i++) {
            send(room->members[i], mode_msg.c_str(), mode_msg.size(), 0);
        }
    }
}
