/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentication.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 00:47:05 by eazmir            #+#    #+#             */
/*   Updated: 2026/04/21 18:53:37 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/utls.hpp"

authentication::authentication():status(false)
{}

authentication::authentication(std::string pass):_serverPassword(pass)
{}

void authentication::handlePass(client &c,const std::string &pass)
{
    if (c.regestred)
        return;
    
    if (pass.empty())
        return;
    if (pass != _serverPassword)
    {
        const char* msg = "464 :Password incorrect\r\n";
        if (send(c.fd, msg, strlen(msg), 0) < 0)
            perror("send");
        return;
    }
    c.pass_ok = true;
}

void authentication::handleUser(client &c,const std::string &user)
{
    if (c.regestred)
        return;
    if (user.empty())
        return;
    if (user != c.username) 
        c.username = user;
    c.user_ok = true;
}

void authentication::handleNick(client &c,const std::string &nick)
{
    if (c.regestred)
        return; 
    if (nick.empty())
        return;
    if (nick != c.nickname)
        c.nickname = nick;
    c.nick_ok = true;
}

std::string authentication::Extract_data(const std::string &data)
{
    size_t pos = data.find(':');
    if (pos == std::string::npos)
        return "";
    return (data.substr(pos + 1));
}

void authentication::tryRegister(client &c, const std::string &input) 
{
    std::stringstream ss(input);
    std::string cmd, arg;
    
    ss >> cmd;
    ss >> arg;

    for (size_t i = 0; i < cmd.size(); i++) cmd[i] = toupper(cmd[i]);

    if (cmd == "PASS") {
        if (arg == _serverPassword) {
            c.pass_ok = true;
        } else {
            std::string err = ":ircserv 464 " + (c.nickname.empty() ? "*" : c.nickname) + " :Password incorrect\r\n";
            send(c.fd, err.c_str(), err.size(), 0);
        }
    } 
    else if (cmd == "NICK") {
        if (!arg.empty()) {
            c.nickname = arg;
            c.nick_ok = true;
        }
    } 
    else if (cmd == "USER") {
        if (!arg.empty()) {
            c.username = arg;
            c.user_ok = true;
        }
    }

    if (c.nick_ok && c.pass_ok && c.user_ok && !c.regestred) {
        c.regestred = true;
        this->send_welcome(c);
    }
}
void authentication::send_welcome(client &c)
{
    std::string msg; 
    msg = std::string(":ircserv 001 ") + c.nickname +
      " :Welcome to the IRC Network, " +
      c.username + "!\r\n" + '\n'; 
    send(c.fd,msg.c_str(),strlen(msg.c_str()),1);
}

void authentication::checkRegistration(client &c)
{
    if (c.nick_ok && c.pass_ok && c.user_ok && status)
    {
        
        this->send_welcome(c);
        c.regestred = true;
        this->status = false;
    }
}