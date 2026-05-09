/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   authentication.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 02:45:56 by eazmir            #+#    #+#             */
/*   Updated: 2026/05/03 19:40:24 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTHENTICATION_HPP
#define AUTHENTICATION_HPP

#include "server.hpp"
#include <map>

struct client;

class authentication
{
    private:
        std::string _serverPassword;
        std::map<int,client> &_clients;
    public:
        authentication(std::map<int,client> &c,std::string &pass);
        int handlePass(client &client, const std::string &pass);
        int handleNick(client &client, const std::string &nick);
        int handleUser(client &client, const std::string &user);
        void tryRegister(client &client,const std::string &input);
        void checkRegistration(client &c);
        std::string Extract_user(const std::vector<std::string> &args);
};
#endif
