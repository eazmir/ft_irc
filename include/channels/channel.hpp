/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 by haitaabe                   #+#    #+#             */
/*   Updated: 2026/04/18 by haitaabe                   ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "../../include/server.hpp"
// #include "../authentication.hpp"

// Forward declaration
struct client;

struct Channel 
{
    std::string       name;
    std::string       topic;
    std::vector<int>  members;      // fds of all members
    std::vector<int>  operators;    // fds of operators
    std::vector<std::string> invite_list;  // nicks invited (for +i mode) 
    std::string       password;     // mode +k
    int               limit;        // mode +l  (0 = no limit)
    bool              invite_only;  // mode +i
    bool              topic_restricted; // mode +t
};

class managerchannel
{
    private:
        std::map<int, client> &_clients;
        std::map<std::string, Channel*> channels;
        std::map<std::string, Channel*>::iterator it; 
        std::vector<std::string> tokens;
        std::string channel_name;
        std::string token;
        Channel *ch;
        std::vector<std::string> splitByComma(const std::string &s);
        // authentication auth;
    public:
        managerchannel(std::map<int, client> &clients,const std::string &pass);
        ~managerchannel();
        void handle_input(const std::string &input, client &c,authentication &auth);
        void handleJoin(const std::string &input, client &c);
        void handlePrivmsg(const std::string &input, client &c);
        void handlePart(const std::string &input, client &c);
        void handleQuit(const std::string &input, client &c);
        void handleKick(const std::string &input, client &c);
        void handleTopic(const std::string &input, client &c);
        void handleInvite(const std::string &input, client &c);
        void handleMode(const std::string &input, client &c);
};

#endif
