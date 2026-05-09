/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utls.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:40:18 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/11 21:40:19 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <iomanip>

class Utils 
{
public:
    static std::string strTrim(const std::string &str);
    static int validatePort(const std::string &port);
    static int check_port(const std::string &port);
    static std::string getTime();
    static std::string getMachineName();
    static void sendHelp(int fd);
    static void send_welcome(client &c);
    static void sendAuthWelcome(client &c);
    static void sendWelcome(int fd);
    static void helpchannel(int fd);
    static std::string to_str(int n);
    static void sendRegistrationHelp(int fd);

};