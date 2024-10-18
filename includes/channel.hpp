/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 13:21:24 by phabets           #+#    #+#             */
/*   Updated: 2023/06/15 15:55:17 by repinat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "client.hpp"

class Channel
{
public :

	Channel();
	Channel(std::string &name);

	~Channel();

	bool	checkGuest(std::string nickname);
	void	addUser(Client &user);
	bool	hasMember(Client &client);
	void	removeMember(Client &client);
	bool	isOperator(std::string nickname);
	void	addOperator(std::string);
	void	removeOperator(std::string);
	bool	checkLimit();
	bool	checkPassword(std::string pass);
	size_t	isPassword();
	
	//getters

	std::string	getTopic() {return _topic;};
	std::string	getName() {return _name;};
	std::map<std::string, Client>::iterator	getIterator();
	std::map<std::string, Client>::iterator	getIterator_end();
	Client	getClient(std::string nick_name);
	bool	getFlagI();
	bool	getFlagT();

	
	//setters

	void	addGuest(std::string nickname) {_guests.push_back(nickname);};
	void	setTopic(std::string topic) {_topic = topic;};
	void	setName(std::string name) {_name = name;};
	void	setOperator(std::string nickname);
	void	setFlagI(bool b);
	void	setFlagT(bool b);
	void	setLimit(size_t l);
	void	setPassword(std::string pass);

private :

	std::vector<std::string> 		_guests;
	std::map<std::string, Client> 	_users;
	std::vector<std::string> 		_operators;
	std::string						_topic;
	std::string						_name;
	std::string						_password;
	
	bool	_i_flag;
	bool	_t_flag;
	size_t	limit;

};

#endif
