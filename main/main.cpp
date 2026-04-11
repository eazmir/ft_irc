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

int	main(int argc, char *argv[])
{
	if (argc != 3)
		return (0);
	server server(std::atoi(argv[1]), argv[2]);
}