/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channels.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 17:13:45 by haitaabe          #+#    #+#             */
/*   Updated: 2026/02/23 17:22:28 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNELS_HPP
#define CHANNELS_HPP



#include <iostream>
#include <string>
#include <map>
#include <vector>




class channels
{
    private:
        std::string CheckChannel;
        std::string Create_A_User;
        bool CheckUser;

    public:
        std::string UserName;
        std::string NickName;

    void CreateChannel(std::string NameOfChannel, int HowManyPpl);
    void PRIVMSG(std::string PrintMessage);
    void JOIN();
    void KICK();
};






#endif