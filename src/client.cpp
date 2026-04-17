/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:41:26 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/16 20:36:52 by haitaabe         ###   ########.fr       */
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
    if (input.compare(0, 7, "PRIVMSG") == 0)
    {
        handlePrivmsg(input, c);
    }
    if (input.compare(0, 4, "PART") == 0)
    {
        handlePart(input, c);
    }
    if (input.compare(0,4,"QUIT") == 0)
    {
        handleQuit(input,c);
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
    // 1. REGISTRATION CHECK
    if (!c.regestred) {
        std::cout << "You are Not regestred !" << std::endl;
        return;
    }

    // 2. PARSING (TOKENIZING)
    std::vector<std::string> local_tokens;
    std::stringstream ss(input);
    std::string token;
    while (ss >> token)
        local_tokens.push_back(token);

    if (local_tokens.size() < 2) {
        std::cout << "Error: Not enough parameters" << std::endl;
        return;
    }
    this->channel_name = local_tokens[1];

    // 3. CHANNEL LOGIC (FIND OR CREATE)
    this->it = channels.find(this->channel_name);
    if (it == channels.end()) {
        Channel *New_ch = new Channel();
        New_ch->name = channel_name;
        // Basic settings for C++98
        New_ch->limit = 0;
        New_ch->invite_only = false;
        New_ch->topic_restricted = false;
        
        channels[this->channel_name] = New_ch;
        this->ch = New_ch;
        this->ch->members.push_back(c.fd);
        this->ch->operators.push_back(c.fd); // First one is the boss
    }
    else {
        this->ch = it->second;
        // Check if user is already a member to avoid duplicates
        bool already_member = false;
        for (size_t i = 0; i < this->ch->members.size(); i++) {
            if (this->ch->members[i] == c.fd) already_member = true;
        }
        if (!already_member)
            this->ch->members.push_back(c.fd);
    }

    // --- PHASE 1: THE BROADCAST (Tell everyone) ---
    std::string prefix = ":" + c.nickname + "!" + c.username + "@localhost";
    std::string join_msg = prefix + " JOIN " + this->channel_name + "\r\n";
    for (size_t i = 0; i < this->ch->members.size(); i++) {
        send(this->ch->members[i], join_msg.c_str(), join_msg.size(), 0);
    }

    // --- PHASE 2: THE TOPIC (Numeric 332) ---
    std::string topic_msg = ":ircserv 332 " + c.nickname + " " + this->channel_name + " :Welcome to " + this->channel_name + "\r\n";
    send(c.fd, topic_msg.c_str(), topic_msg.size(), 0);

    // --- PHASE 3: THE NAMES LIST (Numeric 353 & 366) ---
    std::string names = ":ircserv 353 " + c.nickname + " = " + this->channel_name + " :";
    for (size_t i = 0; i < this->ch->members.size(); i++) {
        int m_fd = this->ch->members[i];
        
        // Check if this member is an operator for the '@' prefix
        bool isOp = false;
        for (size_t j = 0; j < this->ch->operators.size(); j++) {
            if (this->ch->operators[j] == m_fd) isOp = true;
        }

        if (isOp) names += "@";
        names += _clients[m_fd].nickname + " ";
    }
    names += "\r\n";
    send(c.fd, names.c_str(), names.size(), 0);

    // End of Names list
    std::string end_names = ":ircserv 366 " + c.nickname + " " + this->channel_name + " :End of /NAMES list.\r\n";
    send(c.fd, end_names.c_str(), end_names.size(), 0);
}

// to let ppl talk to each other and to know what is the new , private or public 
void managerchannel::handlePrivmsg(const std::string &input, client &c)
{
    std::stringstream scanner(input);
    std::string command, target;

    // 1. Get the basic words
    if (!(scanner >> command >> target)) 
        return; // Safety: Command was too short

    // 2. Find the message (The Scissors)
    size_t pos = input.find(':');
    if (pos == std::string::npos) 
        return; // Safety: No message content found, ignore it.

    std::string message_body = input.substr(pos);

    // 3. Decide where it goes
    if (!target.empty() && target[0] == '#') 
    {
        // --- CHANNEL BROADCAST ---
        std::map<std::string, Channel*>::iterator it = channels.find(target);
        
        if (it != channels.end()) 
        {
            Channel* room = it->second;

            // PREPARE: Build the package once
            std::string final_package = ":" + c.nickname + " PRIVMSG " + target + " " + message_body + "\r\n";

            // DELIVERY: The Loop
            for (size_t i = 0; i < room->members.size(); i++) 
            {
                int recipient_fd = room->members[i];

                // FILTER: No Echo
                if (recipient_fd != c.fd) 
                {
                    send(recipient_fd, final_package.c_str(), final_package.size(), 0);
                }
            }
        }
        else 
        {
            // Error 403: Room not found
            std::string err = ":ircserv 403 " + c.nickname + " " + target + " :No such channel\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    }
    else if (!target.empty())
    {
        // --- PRIVATE MESSAGE (DM) ---
        bool found = false;
        std::map<int, client>::iterator it_client;
        
        for (it_client = _clients.begin(); it_client != _clients.end(); ++it_client)
        {
            if (it_client->second.nickname == target)
            {
                std::string final_package = ":" + c.nickname + " PRIVMSG " + target + " " + message_body + "\r\n";
                send(it_client->first, final_package.c_str(), final_package.size(), 0);
                found = true;
                break; 
            }
        }
        
        if (!found)
        {
            // Error 401: User not found
            std::string err = ":ircserv 401 " + c.nickname + " " + target + " :No such nick\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
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

    // C++98 Cleaner: Remove trailing \r or \n
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
                // Prepare and Broadcast the PART message
                std::string part_msg = ":" + c.nickname + "!" + c.username + "@localhost PART " + target + " " + reason + "\r\n";
                
                for (size_t i = 0; i < room->members.size(); i++)
                {
                    send(room->members[i], part_msg.c_str(), part_msg.size(), 0);
                }

                // The Surgery: remove the user
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