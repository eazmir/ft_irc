/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentication.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 00:47:05 by eazmir            #+#    #+#             */
/*   Updated: 2026/04/28 10:40:02 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/utls.hpp"

authentication::authentication()
{}

authentication::authentication(std::string pass):_serverPassword(pass)
{}


int authentication::handlePass(client &c,const std::string &pass)
{
    std::vector<std::string> args;
    std::stringstream  tokens(pass);
    std::string token;
    //connect <ip> <port> <password>;
    
    while (tokens >> token)
        args.push_back(token);
    
    if (args[1] != c.ip)
    {
        std::string err = ":ircserv 464 * :IP incorrect\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return (0);
    }
    else if (std::atoi(args[2].c_str()) != c.port)
    {
        std::string err = ":ircserv 464 * :Port incorrect\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return (0);
    }
    else  if (args[3] != _serverPassword)
    {
        std::string err = ":ircserv 464 * :Password incorrect\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return (0);
    }
    Utils::sendAuthWelcome(c);
    c.pass_ok = true;
    return (1);
}

int authentication::handleUser(client &c,const std::string &user)
{
    if (c.regestred)
        return (0);
    else if (user.size() > 10)
    {
        std::string err = ":ircserv 432 * USER :Invalid username length (max 10)\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return (0);
    }
    c.username = user;
    c.user_ok = true;
    return (1);
}

int authentication::handleNick(client &c,const std::string &nick)
{
    if (c.regestred)
        return(0);
    else if (nick.size() > 9)
    {
        std::string err = ":ircserv 432 * NICK :Invalid Nickname length (max 9)\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return(0);
    }
    else if (c.nickname == nick)
    {
        std::string err = ":ircserv 433 * " + (c.nickname.empty() ? "*" : c.nickname) + " :Nickname is already in use\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return (0);
    }
    c.nickname = nick;
    c.nick_ok = true;
    return (1);
}

void authentication::tryRegister(client &c,const std::string &input)
{
    std::vector<std::string> arg;
    std::stringstream ss(input);
    std::string name;
    std::string str;
    std::string cmd;

    while (ss >> str)
        arg.push_back(str);

    cmd = arg[0];
    if (cmd.empty())
        return;
    if (arg.size() < 2)
    {
        std::string err = ":ircserv 461 * " + cmd + ": cannot be empty\r\n";
        send(c.fd, err.c_str(), err.size(), 0);
        return;
    }
    else if (cmd == "/user")
    {
        // if (arg.size() < 5)
        // {
        //     std::string err = ":ircserv 461 * USER :Not enough parameters\r\n";
        //     send(c.fd, err.c_str(), err.size(), 0);
        //     return;
        // }
        if (!handleUser(c,arg[1]))
            return;
        name = Extract_user(arg);
        c.realname = name;
    }
    else if (cmd == "/nick")
    {
        if (!handleNick(c,arg[1]))
            return;
    }
    checkRegistration(c);
}

std::string authentication::Extract_user(const std::vector<std::string> &args)
{
    if (args.empty())
        return ("");
    std::string name;
    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i][0] == ':')
        {
            name = args[i].substr(1);
            for (size_t j = i + 1; j < args.size(); j++)
            {
                if (!name.empty())
                    name += " ";
                name += args[j];             
            }
            break;                       
        }
    }
    return (name);
}

void authentication::checkRegistration(client &c)
{
    if (c.nick_ok && c.pass_ok && c.user_ok)
    {
        Utils::send_welcome(c);
        Utils::sendAuthWelcome(c);
        c.regestred = true;
    }
}