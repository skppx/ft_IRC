/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 16:05:45 by repinat           #+#    #+#             */
/*   Updated: 2023/06/06 20:12:42 by repinat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/client.hpp"

Client::Client() :
	_pass_bool(false), _Username_bool(false), _Nickname_bool(false), _registered(false)
{}

Client::~Client() {}

int	Client::isRegistered()
{
	if (_registered)
		return (1);
	return (0);
}


//setters

void	Client::setFd(int fd)
{
	this->_fd = fd;
}

void Client::setRealname(std::string &Realname)
{
	this->_Realname = Realname;
}

void Client::setUsername(std::string &Username)
{
	this->_Username = Username;
}

void Client::setNickname(std::string &Nickname)
{
	this->_Nickname = Nickname;
}

void Client::setBool(int code)
{
	if (code == 0)
		_pass_bool = true;
	else if (code == 1)
		_Nickname_bool = true;
	else
		_Username_bool = true;
}

void Client::setIPaddress(char *IPaddress)
{
	_IPaddress = IPaddress;
}

void Client::setIsRegistered()
{
	_registered = true;
}

//getters

int	Client::getBools()
{
	int	count = 0;
	
	if (_pass_bool)
		count++;
	if (_Nickname_bool)
		count++;
	if(_Username_bool)
		count++;
	
	return (count);
}

int	Client::getFd()
{
	return (this->_fd);
}

std::string	Client::getRealname()
{
	return (this->_Realname);
}

std::string	Client::getUsername()
{
	return (this->_Username);
}

std::string	Client::getNickname()
{
	return (this->_Nickname);
}

std::string& Client::getBuffer()
{
	return (this->_buffer);
}

char*	Client::getIPaddress()
{
	return (this->_IPaddress);
}
