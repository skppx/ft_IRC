/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 14:57:38 by phabets           #+#    #+#             */
/*   Updated: 2023/06/15 15:56:56 by repinat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/channel.hpp"

Channel::Channel() {}

Channel::Channel(std::string &name) : _name(name), _i_flag(false), _t_flag(false), limit(0) {}

Channel::~Channel() {}

bool	Channel::checkGuest(std::string nickname)
{
	for (size_t i = 0; i < _guests.size(); i++)
	{
		if (nickname == _guests[i])
			return true;
	}
	return false;
}


void	Channel::addUser(Client &user)
{
	_users[user.getNickname()] = user;
}

bool Channel::hasMember(Client &client)
{
    std::string nickname = client.getNickname();
	if (_users.count(nickname))
		return true ;
	return false ;

}

bool Channel::isOperator(std::string nickname)
{
	for (std::vector<std::string>::iterator it = _operators.begin(); it != _operators.end(); it++)
	{
		if (*it == nickname)
			return (true);
	}
	return (false);
}

void Channel::addOperator(std::string nickname)
{
	_operators.push_back(nickname);
}
	
void Channel::removeOperator(std::string nickname)
{
	for (std::vector<std::string>::iterator it = _operators.begin(); it != _operators.end(); it++)
	{
		if (*it == nickname)
		{
			_operators.erase(it);
			return;
		}
	}
}

void Channel::removeMember(Client &client)
{
    std::string nickname = client.getNickname();
    _users.erase(nickname);
}

std::map<std::string, Client>::iterator	Channel::getIterator()
{
	std::map<std::string, Client>::iterator it = _users.begin();
	return (it);
}

std::map<std::string, Client>::iterator	Channel::getIterator_end()
{
	std::map<std::string, Client>::iterator it = _users.end();
	return (it);
}

Client	Channel::getClient(std::string nick_name)
{
	return (_users[nick_name]);
}

void	Channel::setOperator(std::string nickname)
{
	_operators.push_back(nickname);
}

bool	Channel::getFlagI()
{
	return (_i_flag);
}

bool	Channel::getFlagT()
{
	return (_t_flag);
}

bool	Channel::checkLimit()
{
	if (limit == 0)
		return false;
	size_t count = _users.size();
	if (count >= limit)
		return true;
	return false;
}

void	Channel::setLimit(size_t l)
{
	limit = l;
}

void	Channel::setFlagI(bool b)
{
	_i_flag = b;
}

void	Channel::setFlagT(bool b)
{
	_t_flag = b;
}

void	Channel::setPassword(std::string pass)
{
	_password = pass;
}

size_t	Channel::isPassword()
{
	return (_password.size());
}

bool	Channel::checkPassword(std::string pass)
{
	if (!(_password.compare(pass)))
		return true;
	return false;
}
