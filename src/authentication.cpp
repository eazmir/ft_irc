/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentication.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eazmir <eazmir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 00:47:05 by eazmir            #+#    #+#             */
/*   Updated: 2026/04/11 15:48:35 by eazmir           ###   ########.fr       */
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

void authentication::tryRegister(client &c,const std::string &input)
{
    // if (input.compare(0,4,"PASS") != 0 && input.compare(0,4,"NICK") != 0 && input.compare(0,4,"USER") != 0)
        // return;
    if (input.compare(0,4,"PASS")== 0)
    {
        handlePass(c,Extract_data(input));
        status = true;
    }
    if (input.compare(0,4,"USER") == 0)
    {
        handleUser(c,Extract_data(input));
        status = true;
    }
    if (input.compare(0,4,"NICK") == 0)
    {
        handleNick(c,Extract_data(input));
        status = true;
    }
    checkRegistration(c);
}
void authentication::send_welcome(client &c)
{
    std::string msg; 
    msg = std::string(":ircserv 001 ") + c.nickname +
      " :Welcome to the IRC Network, " +
      c.username + "!\r\n"; 
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