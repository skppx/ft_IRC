/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/21 18:38:51 by repinat           #+#    #+#             */
/*   Updated: 2023/06/23 17:59:52 by phabets          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.hpp"
#include "../includes/client.hpp"

int	writeMessage(int socket_fd, std::string &message)
{
	if (send(socket_fd, message.c_str(), message.size(), 0) == -1)
	{
		std::cout << "Error while sending message" << std::endl;
		return (-1);
	}
	send(socket_fd, "\r\n", 2, 0);
	return (0);
}

std::vector<std::string> split(const std::string &str)
{
	std::vector<std::string> tokens;
	std::istringstream iss(str);
	std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			std::back_inserter(tokens));
	return tokens;
}

std::vector<std::string> split_delim(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter))
    {
        tokens.push_back(token);
    }
    
    return tokens;
}

void	sig_handler(int sig)
{
	(void)sig;
	IrcServer::flag = false;
}
