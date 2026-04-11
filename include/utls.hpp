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
    static void printClientsInfo(const std::map<int, client> &clients);
};