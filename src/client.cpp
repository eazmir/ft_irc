/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:41:26 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/18 15:44:03 by haitaabe         ###   ########.fr       */
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
    if (input.compare(0, 5, "PRINT") == 0)
        Utils::printClientsInfo(_clients);

    else if (input.compare(0, 5, "JOIN ") == 0 || input == "JOIN") 
    {
        handleJoin(input, c);
    }
    else if (input.compare(0, 5, "PART ") == 0 || input == "PART")
    {
        handlePart(input, c);
    }
    else if (input.compare(0, 5, "QUIT ") == 0 || input == "QUIT")
    {
        handleQuit(input, c);
    }

    else if (input.compare(0, 8, "PRIVMSG ") == 0) 
    {
        handlePrivmsg(input, c);
    }

    else if (input.compare(0, 5, "KICK ") == 0 || input == "KICK") 
    {
        handleKick(input, c);
    }
    else if (input.compare(0, 6, "TOPIC ") == 0 || input == "TOPIC") 
    {
        handleTopic(input, c);
    }
    else if (input.compare(0, 7, "INVITE ") == 0 || input == "INVITE")
    {
        handleInvite(input, c);
    }
    else if (input.compare(0, 5, "MODE ") == 0 || input == "MODE") 
    {
        handleMode(input, c);
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

    std::map<std::string, Channel*>::iterator it;
    for (it = channels.begin(); it != channels.end(); ++it)
    {
        Channel* room = it->second;

        for (std::vector<int>::iterator vit = room->members.begin(); vit != room->members.end(); ++vit)
        {
            if (*vit == c.fd)
            {
                for (size_t i = 0; i < room->members.size(); i++)
                {
                    send(room->members[i], quit_msg.c_str(), quit_msg.size(), 0);
                }

                room->members.erase(vit);

                break; 
            }
        }
    }

    std::cout << "[QUIT] User " << c.nickname << " has left the server." << std::endl;
}


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
    
    for (size_t i = 0; i < modeString.length(); i++) {
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
    }

    if (!appliedModes.empty()) {
        std::string mode_msg = ":" + c.nickname + " MODE " + target + " " + appliedModes + "\r\n";
        for (size_t i = 0; i < room->members.size(); i++) {
            send(room->members[i], mode_msg.c_str(), mode_msg.size(), 0);
        }
    }
}


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