/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 13:27:54 by repinat           #+#    #+#             */
/*   Updated: 2023/06/23 17:58:25 by phabets          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"

int	main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "usage: ./IRC <port> <password>" << std::endl;
		return 0;
	}
	
	IrcServer Server;
	Server.setPort(std::atoi(av[1]));
	Server.setPassword(av[2]);
	Server.setHostname("ircserver");
	// creation du socket
	int	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1)
		return 0;
	Server.setSockfd(sock_fd);
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(Server.getPort());

	//associe le socket a une adresse locale

	if (bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cout << "error: port already taken" << std::endl;
		return (0);

	}
	Server.startServer();

	return (0);
}
