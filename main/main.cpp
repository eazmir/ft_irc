/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/11 21:40:42 by haitaabe          #+#    #+#             */
/*   Updated: 2026/04/11 21:40:43 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/utls.hpp"

int	main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
			throw std::runtime_error("./ircserv <port> <password>");
		server server(Utils::validatePort(argv[1]), argv[2]);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}	
}