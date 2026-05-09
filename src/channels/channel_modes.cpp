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

static bool isPositiveNumber(const std::string &value)
{
    if (value.empty())
        return false;
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(value[i]))
            return false;
    }
    return true;
}

static void sendModeError(client &c, const std::string &target, const std::string &text)
{
    std::string err = ":ircserv 696 " + c.nickname + " " + target + " :" + text + "\r\n";
    send(c.fd, err.c_str(), err.size(), 0);
}

static std::string buildChannelModes(Channel *room)
{
    std::string modes = "+";
    std::string params;

    if (room->invite_only)
        modes += "i";
    if (room->topic_restricted)
        modes += "t";
    if (!room->password.empty())
    {
        modes += "k";
        params += " " + room->password;
    }
    if (room->limit > 0)
    {
        modes += "l";
        params += " " + Utils::to_str(room->limit);
    }
    return modes + params;
}

void managerchannel::handleMode(const std::string &input, client &c) {
    std::stringstream ss(input);
    std::string command, target;
    std::vector<std::string> args;

    while (ss >> command)
        args.push_back(command);

    if (args.size() < 2)
        return;

    command = args[0];
    target = args[1];

    if (target.empty() || target[0] != '#')
        return; 
    
    if (channels.find(target) == channels.end()) {
        std::string err = ":ircserv 403 " + c.nickname + " " + target + " :No such channel\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    Channel *room = channels[target];

    if (!c.regestred)
    {
        std::string err = ":ircserv 451 " + (c.nickname.empty() ? "*" : c.nickname) + " :You have not registered\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }

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

    if (args.size() == 2)
    {
        std::string mode_msg = ":ircserv 324 " + c.nickname + " " + target + " " + buildChannelModes(room) + "\r\n";
        send(c.fd, mode_msg.c_str(), mode_msg.size(), 0);
        return;
    }

    std::string modeString = args[2];
    std::vector<std::string> params;
    for (size_t i = 3; i < args.size(); ++i)
        params.push_back(args[i]);

    if (modeString.empty())
    {
        sendModeError(c, target, "Invalid mode string");
        return;
    }

    Channel updatedRoom = *room;
    size_t paramIndex = 0;
    bool adding = true;
    bool sawMode = false;

    for (size_t i = 0; i < modeString.length(); ++i)
    {
        char mode = modeString[i];
        if (mode == '+') { adding = true; continue; }
        if (mode == '-') { adding = false; continue; }

        sawMode = true;

        if (mode == 'i')
        {
            updatedRoom.invite_only = adding;
        }
        else if (mode == 't')
        {
            updatedRoom.topic_restricted = adding;
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (paramIndex >= params.size())
                {
                    sendModeError(c, target, "Not enough parameters");
                    return;
                }
                updatedRoom.password = params[paramIndex++];
                if (updatedRoom.password.empty())
                {
                    sendModeError(c, target, "Invalid key value");
                    return;
                }
            }
            else
            {
                if (paramIndex < params.size())
                {
                    sendModeError(c, target, "Too many parameters");
                    return;
                }
                updatedRoom.password.clear();
            }
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (paramIndex >= params.size())
                {
                    sendModeError(c, target, "Not enough parameters");
                    return;
                }
                std::string limitValue = params[paramIndex++];
                if (!isPositiveNumber(limitValue) || std::atoi(limitValue.c_str()) <= 0)
                {
                    sendModeError(c, target, "Invalid limit value");
                    return;
                }
                updatedRoom.limit = std::atoi(limitValue.c_str());
            }
            else
            {
                if (paramIndex < params.size())
                {
                    sendModeError(c, target, "Too many parameters");
                    return;
                }
                updatedRoom.limit = 0;
            }
        }
        else if (mode == 'o')
        {
            if (paramIndex >= params.size())
            {
                sendModeError(c, target, "Not enough parameters");
                return;
            }

            std::string nick = params[paramIndex++];
            int target_fd = -1;
            for (std::map<int, client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
            {
                if (it->second.nickname == nick)
                {
                    target_fd = it->first;
                    break;
                }
            }

            if (target_fd == -1)
            {
                sendModeError(c, target, "No such nick/channel");
                return;
            }

            if (adding)
            {
                bool already_op = false;
                for (size_t j = 0; j < updatedRoom.operators.size(); j++)
                    if (updatedRoom.operators[j] == target_fd)
                        already_op = true;
                if (!already_op)
                    updatedRoom.operators.push_back(target_fd);
            }
            else
            {
                for (std::vector<int>::iterator oit = updatedRoom.operators.begin(); oit != updatedRoom.operators.end(); ++oit)
                {
                    if (*oit == target_fd)
                    {
                        updatedRoom.operators.erase(oit);
                        break;
                    }
                }
            }
        }
        else
        {
            sendModeError(c, target, "Unknown mode flag");
            return;
        }
    }

    if (!sawMode)
    {
        sendModeError(c, target, "Invalid mode string");
        return;
    }

    if (paramIndex != params.size())
    {
        sendModeError(c, target, "Too many parameters");
        return;
    }

    *room = updatedRoom;

    std::string appliedModes = modeString;
    for (size_t i = 0; i < params.size(); ++i)
    {
        appliedModes += " ";
        appliedModes += params[i];
    }

    std::string mode_msg = ":" + c.nickname + " MODE " + target + " " + appliedModes + "\r\n";
    for (size_t i = 0; i < room->operators.size(); i++) {
        send(room->operators[i], mode_msg.c_str(), mode_msg.size(), 0);
    }
}
