/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: repinat <repinat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/25 15:30:47 by repinat           #+#    #+#             */
/*   Updated: 2023/06/26 13:37:14 by phabets          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../includes/server.hpp"
#include "../includes/client.hpp"

bool IrcServer::flag = true;

IrcServer::IrcServer() {};

IrcServer::~IrcServer() {};

// void IrcServer::sig_handler(int sig)
// {
// 	(void)sig;
// 	for (int i = 3; i < 1025; i++)
// 		close(i);
// 	fds.clear();
// 	exit(1);
// }
//


//void (*wrapper(IrcServer *serv))(int sig) {
//	return (reinterpret_cast<void (int)>(&(serv->sig_handler)));
//}

void	IrcServer::startServer()
{
	std::cout << "Server started..." << std::endl;
	if (listen(getSockfd(), 10) == -1)
	{
		std::cerr << "error: listen" << std::endl;
		return ;
	}
	signal(SIGINT, sig_handler);
	struct pollfd new_fd;
	new_fd.fd = _sock_fd;
	new_fd.events = POLLIN;
	fds.push_back(new_fd);
	_nfds = 1;
	
	while (IrcServer::flag)
	{
		// signal(SIGINT, sig_handler);
		int ready = poll(&fds.front(), _nfds, -1);
		if (ready == 0)
			std::cerr << "Timeout" << std::endl;
		if (ready > 0)
		{
			if (fds[0].revents == POLLIN)
				this->newClient();
			else
			{
				for(size_t i = 1; i < _nfds; i++)
				{
					if (fds[i].revents == POLLIN)
						this->newCommand(fds[i].fd, i);
				}
			}
		}
	}

}
//cree un nouveau objet client, assigne son fd, et le push
//sous forme de std::map avec son fd comme key dans
//"std::map<int, Client> _clients" contenu dans IrcServer
void	IrcServer::newClient()
{
	//marque le socket comme "pret" a accueillir des connexions entrantes
	// std::cout << "NEW CLIENT" <<std::endl;
	int test_fd;
	test_fd = accept(_sock_fd, NULL, NULL);
	struct pollfd new_fd;
	new_fd.fd = test_fd;
	new_fd.events = POLLIN;
	fds.push_back(new_fd);
	this->_nfds++;
	Client	newClient;
	
	newClient.setFd(test_fd);
	
	//recuperer adresse ip du client
	struct	sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(test_fd, (struct sockaddr *)&addr, &addr_size);
	// (void)res;
	if (res == -1)
	{
		std::cerr << "Error: getpeername" << std::endl;
		return ;
	}
	char *ip_str = inet_ntoa(addr.sin_addr);
	newClient.setIPaddress(ip_str);
	
	//inserer le nouveau client dans la map
	newClient.setIsRegistered();
	this->_clients[test_fd] = newClient; 
}

void	IrcServer::newCommand(int fd, int i)
{
	// std::cout << "New command" << std::endl;
	char buffer[512];
	size_t count = 512;
	int	len;
	std::string buffer_ret;

	if ((len = read(fd, buffer, count)) == 0)
	{
		_nfds--;
		fds.erase(fds.begin()+i);
		disconnectClient(fd, _clients, _channels);
		close(fd);
	}
	else 
	{
		buffer[len] = '\0';
		_clients[fd].getBuffer().append(buffer, len);
		if (_clients[fd].getBuffer().find('\012') != std::string::npos)
		{
			buffer_ret = _clients[fd].getBuffer();
			parsing(buffer_ret , fd);
			_clients[fd].getBuffer().clear();
		}
	}
}

//recoit ce que le client envoie et split tous les '\n' et range dans un std::vector
void	IrcServer::parsing(std::string &command, int fd)
{
	std::vector<std::string> words;
	
	std::string::size_type	startPos = 0;
	std::string::size_type	bnPos;

	while ((bnPos = command.find_first_of("\n\r", startPos)) != std::string::npos)
	{
		words.push_back(command.substr(startPos, (bnPos - startPos)));
		startPos = bnPos + 1;
	}
	words.push_back(command.substr(startPos));
	
	for (size_t i = 0; i < words.size(); i++)
		sortingWords(words[i], fd);
}

//recoit chaque iteration du vector de la fonction ci dessus et
//split tous les ' ' dans un nouveau vector
void	IrcServer::sortingWords(std::string &word, int fd)
{
	std::vector<std::string> splitted;
	
	std::string::size_type	startPos = 0;
	std::string::size_type	spacePos;
	std::string message;

	//parsing pour PRIVMSG
	if ((!word.find("PRIVMSG", 0, 7)) || ((word.find(":") != std::string::npos) && (!word.find("TOPIC", 0, 5))))
	{
		spacePos = word.find(' ', startPos);
		splitted.push_back(word.substr(startPos, (spacePos - startPos)));
		startPos = spacePos + 1;
		std::string::size_type	tmpPos = word.find(':', startPos);
		splitted.push_back(word.substr(startPos, (tmpPos - startPos - 1)));
		splitted.push_back(word.substr(tmpPos + 1, std::string::npos));
	}
	// else if(!word.compare("WHO"))
	// {
		//pouvoir aller directement dans la fonction who si WHO est passee sans arguments
		// splitted.push_back(word);	
		// command_who(splitted, fd);
	// }
	splitted = split(word);
	if (splitted.size() >= 2)
		parsingCommand(splitted, fd);
}

//traite la commande
void	IrcServer::parsingCommand(std::vector<std::string> &command, int fd)
{
	// for (size_t i = 0; i < command.size(); i++)
	// 	std::cout << "parsing : COMMAND [" << i << "] = " << command[i] << std::endl;
	if (command.size() > 2 && command[2].find(' ', 0) == 0)
	{
		std::string::size_type	endPos = 0;
		while (!command[2].compare(" "))
			endPos++;
		command.erase(command.begin(), command.begin() + endPos);
	}
	if (!command[0].compare("PASS"))
		command_pass(command[1], fd);
	if (_clients[fd].getBools() == 1)
	{
		if (!command[0].compare("NICK"))
			command_nick(command[1], fd);
	}
	if (_clients[fd].getBools() == 2)
	{
		if (!command[0].compare("USER"))
			command_user(command, fd);
	}
	if (_clients[fd].isRegistered())
	{
		if (!command[0].compare("PING"))
			command_pong(fd);
		else if (!command[0].compare("PRIVMSG"))
			command_prvmsg(command, fd);
		else if (!command[0].compare("JOIN"))
			command_join(command, fd);
		else if (!command[0].compare("PART"))
			command_part(command, fd);
		// else if (!command[0].compare("WHO"))
		// 	command_who(command, fd);
		else if (!command[0].compare("TOPIC"))
			command_topic(command, fd);
		else if (!command[0].compare("MODE"))
			command_mode(command, fd);
		else if (!command[0].compare("INVITE"))
			command_invite(command, fd);
		else if (!command[0].compare("KICK"))
			command_kick(command, fd);
	}
}

void IrcServer::disconnectClient(int fd, std::map<int, Client> &_clients, std::map<std::string, Channel> &_channels)
{
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
		it->second.removeMember(_clients[fd]);
	_clients.erase(fd);
}

// ------------------- COMMANDES -----------------------

void	IrcServer::command_pong(int fd)
{
	std::string response;

	response = "PONG :" + this->getHostname();
	writeMessage(fd, response);
}

void	IrcServer::command_pass(std::string &mdp, int fd)
{
	if (!mdp.compare(getPassword()))
		_clients[fd].setBool(0);
}

void	IrcServer::command_nick(std::string &nick, int fd)
{
	std::string message;
	if (nick.size() > 9)
	{
		message = ":" + getHostname() + " " + ERR_ERRONEUSNICKNAME + " " + nick + " " + ":Eroneous nickname";
		writeMessage(fd,message);
		return;
	}
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (!it->second.getNickname().empty())
		{
			if (!it->second.getNickname().compare(nick))
			{
				message = ":" + getHostname() + " " + ERR_NICKNAMEINUSE + " " + nick + " " + ":Nickname is already in use";
				writeMessage(fd,message);
				return;
			}
		}
	}
	_clients[fd].setNickname(nick);
	_clients[fd].setBool(1);
}

int	IrcServer::command_user(std::vector<std::string> &command, int fd)
{
	std::string	response;
	std::string nick = _clients[fd].getNickname();
	std::string	realname;
	

	//si nick n'a pas encore ete entre par le client
	if (_clients[fd].getBools() != 2)
	{
		response = ":" + this->getHostname() + " " + ERR_NOTREGISTERED + " " + command[1] + " :You have not registered";
		if (writeMessage(fd, response) == -1)
			return (EXIT_FAILURE);
	}
	//si la commande n'a pas recu assez d'arguments 
	else if (command.size() < 5)
	{
		response = ":" + this->getHostname() + " " + ERR_NEEDMOREPARAMS + " " + nick + " USER: Not enough parameters";
		if (writeMessage(fd, response) == -1)
			return (EXIT_FAILURE);
	}
	//si la commande est entree correctement 
	//passer le booleen a 3 des que le client est enregistre
	else if (_clients[fd].getBools() != 3)
	{
		_clients[fd].setUsername(command[1]);
		
		//gerer realname command[4]
		for (size_t i = 4; i < command.size(); i++)
		{
			if (i == 4)
				realname = command[i].substr(1, command[i].size() - 1);
			else
			{
				realname += " ";
				realname += command[i];	
			}
		}
		size_t i = 0;
		while (realname[i] == ' ')
			i++;
		if (i != 0)
			realname.erase(0, i);
		// if (realname.empty() == 0)
		// 	realname.erase(realname.size() - 1);
		_clients[fd].setRealname(realname);
		_clients[fd].setBool(2);
		_clients[fd].setUsername(command[1]);
		response = ":" + this->getHostname() + " " + RPL_WELCOME + " " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + command[1] + "@" + _clients[fd].getIPaddress() + "\r\n";
		response += ":" + this->getHostname() + " " + RPL_YOURHOST + " " + nick + ":Your host is " + this->getHostname() + ", running version 1.0\r\n";
		response += ":" + this->getHostname() + " " + RPL_CREATED + " " + nick + " :This server was created in 2023\r\n";
		response += ":" + this->getHostname() + " " + RPL_MYINFO + " " + nick + " " + getHostname() + " 1.0 itkol";

		if (writeMessage(fd, response) == -1)
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}
	//si le client est deja enregistre
	else
	{
		//changer servename
		response = std::string(":servername ") + ERR_ALREADYREGISTRED + std::string(" ") + nick
			+ " :Unauthorized command (already registered)";
		if (writeMessage(fd, response) == -1)
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void	IrcServer::command_prvmsg(std::vector<std::string> &arg, int fd)
{
	int fd_receiver = -1;
	std::string tmp;
	std::string message;
	std::string response;
	std::vector<std::string>::iterator it = arg.begin() + 1;
	
	//si il n'y a pas de message a envoyer
	if (arg.size() < 3)
	{
		response = ":" + this->getHostname() + " " + ERR_NOTEXTTOSEND + " " + _clients[fd].getNickname() + " :No text to send";
		writeMessage(fd, response);
	}
	//message pour channel
	if (it->find_first_of("#&") != std::string::npos)
	{
		// std::cout << *it << std::endl;
		std::string chan_name = it->substr(it->find_first_of("#&") + 1, it->find_last_of(","));
		//si le channel specifie n'existe pas 
		if (!_channels.count(chan_name))
		{
			response = ":" + this->getHostname() + " " + ERR_NOSUCHCHANNEL + " " + _clients[fd].getNickname() + " " + arg[1] + " :No such channel";
			writeMessage(fd, response);
			return ;
		}
		if (_channels.count(chan_name) && _channels[chan_name].hasMember(_clients[fd]))
		{
			std::map<std::string, Client>::iterator user_it;
			for(user_it = _channels[chan_name].getIterator(); user_it != _channels[chan_name].getIterator_end(); ++user_it)
			{
				fd_receiver = user_it->second.getFd();
				if (fd_receiver != fd)
				{
					message = ":" + _clients[fd].getNickname() + "!"
						+ _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress();
					for (size_t i = 0; i < arg.size(); i++)
					{
						message += " ";
						message += arg[i];
					}
					writeMessage(fd_receiver, message);
				}
			}
		}
		//si le client qui envoi le message n'est pas membre du channel
		else
		{
			response = ":" + this->getHostname() + " " + ERR_CANNOTSENDTOCHAN + " " + _clients[fd].getNickname() + " #" + chan_name + " :Cannot send to channel";
			writeMessage(fd, response);
			return ;
		}
	}
	else
	{
		//message pour client	
		if (getClientByNickname(arg[1]))
		{
			fd_receiver = getClientByNickname(arg[1])->getFd();
			message = ":" + _clients[fd].getNickname() + "!"
				+ _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress();
			for (size_t i = 0; i < arg.size(); i++)
			{
				message += " ";
				message += arg[i];
			}
			writeMessage(fd_receiver, message);
		}
		//si le client specifie n'existe pas 
		else
		{
			response = ":" + this->getHostname() + " " + ERR_NOSUCHNICK + " " + _clients[fd].getNickname() + " " + arg[1] + " :No such nick";
			writeMessage(fd, response);	
		}
	}
}

void	IrcServer::command_join(std::vector<std::string> &arg, int fd)
{
	std::string response;
	std::string chan_name;
	std::string message = "Channel not found";
	std::vector<std::string>::iterator it = arg.begin() + 1;

	if (arg.size() < 2)
	{
		response = ":" + this->getHostname() + " " + ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + " JOIN :Not enough parameters";
		writeMessage(fd, response);
		return ;
	}
	while(it != arg.end())
	{
		if (it->find_first_of("#&") != std::string::npos)
		{
			/*----------------------------------------------------------*/
			//check de la syntaxe du nom de channel
			chan_name = it->substr(it->find_first_of("#&") + 1, it->find_last_of(","));
			for (size_t i = 0; i < chan_name.size(); i++)
			{
				if (chan_name[i] >= 0 && chan_name[i] <= 31)
				{
					response = ":" + this->getHostname() + " " + ERR_BADCHANMASK + " " + _clients[fd].getNickname() + "#" + chan_name + " :Bad Channel Mask";
					writeMessage(fd, response);
					return ;
				}
			}
			/*----------------------------------------------------------*/
			if (_channels.count(chan_name))
			{
				if (_channels[chan_name].getFlagI())
				{
					if (_channels[chan_name].checkGuest(_clients[fd].getNickname()))
					{
						response = ":" + _clients[fd].getNickname() + " JOIN #" + chan_name;
						_channels[chan_name].addUser(_clients[fd]);
						std::map<std::string, Client>::iterator it;
						for(it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
						{
							if (_channels[chan_name].hasMember(it->second))
							writeMessage(it->second.getFd(), response);
						}
						if (_channels[chan_name].getTopic().empty())
							response = ":" + this->getHostname() + " " + RPL_NOTOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :No topic is set";	
						else
							response = ":" + this->getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " " + _channels[chan_name].getTopic();
						writeMessage(fd, response);
					}
					else
					{
						message = ":" + this->getHostname() + " " + ERR_INVITEONLYCHAN + " " + _clients[fd].getNickname() + " #" + chan_name + " :Cannot join channel (+i)";
						writeMessage(fd, message);
					}
				}
				else if (_channels[chan_name].checkLimit())
				{
					message = ":" + this->getHostname() + " " + ERR_CHANNELISFULL + " " + _clients[fd].getNickname() + " #" + chan_name + " :Cannot join channel (+l)";
					writeMessage(fd, message);

				}
				else if (_channels[chan_name].isPassword())
				{

					if ((it + 1) != arg.end() && _channels[chan_name].checkPassword(*(it + 1))) // checker la taille pour savoir si il y'a un mdp
					{
						response = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress()
							+ " " + std::string("JOIN") + " :#" + chan_name;
						_channels[chan_name].addUser(_clients[fd]);
						std::map<std::string, Client>::iterator it;
						for(it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
						{
							if (_channels[chan_name].hasMember(it->second))
							writeMessage(it->second.getFd(), response);
						}
						if (_channels[chan_name].getTopic().empty())
							response = ":" + this->getHostname() + " " + RPL_NOTOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :No topic is set";	
						else
							response = ":" + this->getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :" + _channels[chan_name].getTopic();
						writeMessage(fd, response);
					}
					else
					{
						response = ":" + this->getHostname() + " " + ERR_BADCHANNELKEY + " " + _clients[fd].getNickname() + " #" + chan_name + " :Cannot join channel (+k)";
						writeMessage(fd, response);
					}
				}
				else
				{
					response = ":" + _clients[fd].getNickname() + " JOIN #" + chan_name;
					_channels[chan_name].addUser(_clients[fd]);
					std::map<std::string, Client>::iterator it;
					for(it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
					{
						if (_channels[chan_name].hasMember(it->second))
						writeMessage(it->second.getFd(), response);
					}
					if (_channels[chan_name].getTopic().empty())
						response = ":" + this->getHostname() + " " + RPL_NOTOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :No topic is set";	
					else
						response = ":" + this->getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :" + _channels[chan_name].getTopic();
					writeMessage(fd, response);
				}
			}
			else if (!_channels.count(chan_name))
			{
				_channels[chan_name] = Channel(chan_name);
				_channels[chan_name].addUser(_clients[fd]);
				_channels[chan_name].setOperator(_clients[fd].getNickname());
				response = ":" + _clients[fd].getNickname() + " JOIN #" + chan_name;
				writeMessage(fd, response);
				if (_channels[chan_name].getTopic().empty())
					response = ":" + this->getHostname() + " " + RPL_NOTOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :No topic is set";	
				else
					response = ":" + this->getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname() + " #" + chan_name + " :" + _channels[chan_name].getTopic();
				writeMessage(fd, response);
			}
			else
				writeMessage(fd, message);
		}
		// message = ":" + getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname()
		// 	+ " " + arg[1] + " " + ":" + topic;
		it++;
	}
}

void IrcServer::command_modei(char sign, int fd, std::vector<std::string> &command, std::string chan_name)
{
	std::string message;
	if (sign == '+')
	{
		_channels[chan_name].setFlagI(true);
		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +i";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2];
		writeMessage(fd, message);
	}
	else if (sign == '-')
	{
		_channels[chan_name].setFlagI(false);
		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " -i";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2];
		writeMessage(fd, message);
	}
	else
	{
		std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
			+ " " + command[2] + " " + sign + " :is unknown mode char to me";
		writeMessage(fd, message);
	}
}

void IrcServer::command_modet(char sign, int fd, std::vector<std::string> &command, std::string chan_name)
{
	std::string message;
	if (sign == '+')
	{
		_channels[chan_name].setFlagT(true);
		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +t";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2];
		writeMessage(fd, message);
	}
	else if (sign == '-')
	{
		_channels[chan_name].setFlagT(false);
		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " -t";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2];
		writeMessage(fd, message);
	}
	else
	{
		std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
			+ " " + command[2] + " " + sign + " :is unknown mode char to me";
		writeMessage(fd, message);
	}
}

void IrcServer::command_modek(char sign, int fd, std::vector<std::string> &command, std::string chan_name)
{
	std::string message;
 	if (sign == '+')
 	{
 		_channels[chan_name].setPassword(command[3]);
 		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +k " + command[3];
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2] + " " + command[3];
		writeMessage(fd, message);
	}
	else if (sign == '-')
	{
		_channels[chan_name].setPassword("");
 		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " -k";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
		message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2];
		writeMessage(fd, message);
	}
	else
	{
		std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
			+ " " + command[2] + " " + sign + " :is unknown mode char to me";
		writeMessage(fd, message);
	}
}

void IrcServer::command_modeo(char sign, int fd, std::vector<std::string> &command, std::string chan_name)
{
	std::string message;
	if (sign == '+')
	{
		if (_channels[chan_name].hasMember(*getClientByNickname(command[3])))
		{
			_channels[chan_name].addOperator(command[3]);
			message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +o " + command[3];
			for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
				writeMessage(it->second.getFd(), message);
			message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2] + " " + command[3];
			writeMessage(fd, message);
		}
		else
		{
			message = ":" + this->getHostname() + " " + ERR_USERNOTINCHANNEL + " " + _clients[fd].getNickname() + " " + command[3] +
				" #" + chan_name + " :User not in channel";
			writeMessage(fd, message);
		}
				//notinchannel
	}
	else if (sign == '-')
	{
		if (_channels[chan_name].hasMember(*getClientByNickname(command[3])))
		{
			_channels[chan_name].removeOperator(command[3]);
			message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +o " + command[3];
			for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
				writeMessage(it->second.getFd(), message);
			message = ":" + this->getHostname() + " " + RPL_CHANNELMODEIS + " " + _clients[fd].getNickname() + " #" + chan_name + " " + command[2] + " " + command[3];
			writeMessage(fd, message);
		}
		else
		{
			message = ":" + this->getHostname() + " " + ERR_USERNOTINCHANNEL + " " + _clients[fd].getNickname() + " " + command[3] +
				" #" + chan_name + " :User not in channel";
			writeMessage(fd, message);
				//notinchannel
		}
	}
	else
	{
		std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
			+ " " + command[2] + " " + sign + " :is unknown mode char to me";
		writeMessage(fd, message);
		//error
	}
}
void IrcServer::command_model(char sign, int fd, std::vector<std::string> &command, std::string chan_name)
{
	std::string message;
	if (command.size() < 3)
	{
		message = ":" + this->getHostname() + " " ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + "MODE (+l) :Not enough parameters";
		writeMessage(fd, message);
		return;
	}	
	for (std::string::iterator it = command[3].begin(); it != command[3].end(); ++it)
		if (!std::isdigit(*it))
		{
			message = ":" + this->getHostname() + " " ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + "MODE (+l) :parameters is nondigit";
			writeMessage(fd, message);
		}
	if (sign == '+')
	{
		_channels[chan_name].setLimit(std::atoi(command[3].c_str()));
 		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " +l" + command[3];
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
	}
	else if (sign == '-')
	{
		_channels[chan_name].setLimit(0);
 		message = ":" + _clients[fd].getNickname() + " MODE #" + chan_name + " -l";
		for(std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
			writeMessage(it->second.getFd(), message);
	}
	else
	{
		std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
			+ " " + command[2] + " " + sign + " :is unknown mode char to me";
		writeMessage(fd, message);
		//error
	}
}

void IrcServer::command_mode(std::vector<std::string> &command, int fd)
{
	std::string chan_name;
	std::string message;
	char sign;

	if (command[1][0] == '#' || command[1][0] == '&')
	{
		chan_name = command[1].substr(1);
		if (_channels.count(chan_name))
		{
			if (_channels[chan_name].isOperator(_clients[fd].getNickname()))
			{
				std::string::iterator it = command[2].begin();
				sign = *it;
				it++;
				if (*it == 'i')
					command_modei(sign, fd, command, chan_name);
				else if (*it == 't')
					command_modet(sign, fd, command, chan_name);
				else if (*it == 'k')
					command_modek(sign, fd, command, chan_name);
				else if (*it == 'o')
					command_modeo(sign, fd, command, chan_name);
				else if (*it == 'l')
					command_model(sign, fd, command, chan_name);
				else
				{
					std::string message = ":" + getHostname() + " " + ERR_UNKNOWNMODE + " " + _clients[fd].getNickname()
						+ " " + command[2] + " " + sign + " :is unknown mode char to me";
					writeMessage(fd, message);
				}
			}
			else
			{
				message = ":" + getHostname() + " " + ERR_CHANOPRIVSNEEDED + " " + _clients[fd].getNickname()
					+ " " + command[1] + " :You're not channel operator";
				writeMessage(fd, message);

			}
		}
		else
		{
			message = ":" + getHostname() + " " + ERR_NOSUCHCHANNEL + " " + _clients[fd].getNickname()
				+ " " + command[1] + " " + ":No such channel";
			writeMessage(fd, message);
		}
	}
	else
	{
			message = ":" + getHostname() + " " + ERR_UNKNOWNCOMMAND + " " + _clients[fd].getNickname()
				+ " " + command[1] + " " + ":User mode not handeled";
			writeMessage(fd, message);
	}
}

void IrcServer::command_part(std::vector<std::string> &command, int fd)
{
	std::string	response;
	std::string part_msg;
	std::string	chan_name;

	if (command.size() < 1)
	{
		response = ":" + this->getHostname() + " " ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + "PART :Not enough parameters";
		writeMessage(fd, response);
		return ;
	}
	else 
	{
		chan_name = command[1].substr(1, std::string::npos);
		//si le nom du channel de la commande PART est correct
		if (_channels.count(chan_name))
		{
			//si le client qui utilise la commande n'est pas membre du channel
			if (!_channels[chan_name].hasMember(_clients[fd]))
			{
				response = ":" + this->getHostname() + " " + ERR_NOSUCHCHANNEL + " " + _clients[fd].getNickname() + " " + chan_name + " :No such channel";
				writeMessage(fd, response);	
				return ;
			}
			//si le client qui utilise la commande est membre du channel
			else
			{
				_channels[chan_name].removeMember(_clients[fd]);
				response = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress() + " PART #" + chan_name;
				writeMessage(fd, response);
				if (command.size() == 2)
					part_msg = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress() + " PART #" + chan_name;
				else if (command.size() > 3)
				{
					part_msg = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress() + " PART #" + chan_name;
					part_msg += " ";
					for (size_t i = 2; i < command.size(); i++)
					{
						part_msg += command[i];
						if (i != command.size() - 1)
							part_msg += " ";
					}
				}
				
				std::map<std::string, Client>::iterator it;
				for(it = _channels[chan_name].getIterator(); it != _channels[chan_name].getIterator_end(); ++it)
				{
				    if (_channels[chan_name].hasMember(it->second))
    			    writeMessage(it->second.getFd(), part_msg);
				}
				return ;
			}
		}
	}
}

void	IrcServer::command_topic(std::vector<std::string> &arg, int fd)
{
	std::string chan_name = arg[1].substr(arg[1].find_first_of("&#") + 1, std::string::npos);
	std::string message;
	std::string topic;
	std::string nickname;

	nickname = _clients[fd].getNickname();
	if (arg.size() <= 1)
	{
		message = ":" + getHostname() + " " + ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname()
			+ " " + arg[0] + " " + ":Not enough parameters";
		writeMessage(fd,message);
	}
	else if (!_channels.count(chan_name))
	{
		message = ":" + getHostname() + " " + ERR_NOSUCHCHANNEL + " " + _clients[fd].getNickname()
			+ " " + arg[1] + " " + ":No such channel";
		writeMessage(fd, message);
	}
	else if (_channels[chan_name].getClient(nickname).getFd() != fd) //error not in channel
	{
		message = ":" + getHostname() + " " + ERR_NOTONCHANNEL + " " + _clients[fd].getNickname()
			+ " " + arg[1] + " " + ":You're not on that channel";
		writeMessage(fd, message);
	}
	else
	{
		topic = _channels[chan_name].getTopic();
		if (arg.size() == 2)
		{
			if (!topic.empty())
			{
				message = ":" + getHostname() + " " + RPL_TOPIC + " " + _clients[fd].getNickname()
					+ " " + arg[1] + " " + " :" + topic;
				writeMessage(fd, message);
			}
			else
			{
				message = ":" + getHostname() + " " + RPL_NOTOPIC + " " + _clients[fd].getNickname()
					+ " " + arg[1] + " " + ":No topic is set";
				writeMessage(fd, message);
			}

		}
		else if (arg.size() >= 3)
		{
			if (_channels[chan_name].getFlagT())
			{
				if (_channels[chan_name].isOperator(_clients[fd].getNickname()))
				{
					std::string topic = arg[2].substr(arg[2].find_first_of(":") + 1, std::string::npos);
					if (arg.size() > 3)
					{
						for (size_t i = 3; i < arg.size(); i++)
						{
							topic += " ";
							topic += arg[i];
						}
					}
					_channels[chan_name].setTopic(topic);
					std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator();
					while (it != _channels[chan_name].getIterator_end())
					{
						message = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress()
							+ " " + std::string("TOPIC") + " " + arg[1] + " :" + topic;
						writeMessage(it->second.getFd(), message);
						it++;
					}
				}
				else
				{
					message = ":" + getHostname() + " " + ERR_CHANOPRIVSNEEDED + " " + _clients[fd].getNickname()
						+ " #" + chan_name + " :You're not channel operator";
					writeMessage(fd, message);
				}
			}
			else
			{
				std::string topic = arg[2].substr(arg[2].find_first_of(":") + 1, std::string::npos);
				if (arg.size() > 3)
				{
					for (size_t i = 3; i < arg.size(); i++)
					{
						topic += " ";
						topic += arg[i];
					}
				}
				_channels[chan_name].setTopic(topic);
				std::map<std::string, Client>::iterator it = _channels[chan_name].getIterator();
				while (it != _channels[chan_name].getIterator_end())
				{
					message = ":" + _clients[fd].getNickname() + "!~" + _clients[fd].getUsername() + "@" + _clients[fd].getIPaddress()
						+ " " + std::string("TOPIC") + " " + arg[1] + " :" + topic;
					writeMessage(it->second.getFd(), message);
					it++;
				}
			}
		}
	}
}

void	IrcServer::command_invite(std::vector<std::string> &command, int fd)
{
	std::string	response;
	std::string	nick_target = command[1];
	std::string	channel_target = command[2].substr(1, std::string::npos);

	//si il n'y a pas le bon nombre d'arguments
	if (command.size() != 3)
	{
		response = ":" + this->getHostname() + " " + ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + " INVITE: Not enough parameters";
		writeMessage(fd, response);
		return ;
	}
	//si le nickname de la personne invitee n'existe pas
	if (!getClientByNickname(nick_target))
	{
		response = ":" + this->getHostname() + " " + ERR_NOSUCHNICK + " " + nick_target + " :No such nick";	
		writeMessage(fd, response);
		return ;
	}
	//si le nom du channel specifie n'existe pas
	if (!_channels.count(channel_target))
	{
		response = ":" + this->getHostname() + " " + ERR_NOSUCHCHANNEL + " " + nick_target + " #" + channel_target + " :No such channel";
		writeMessage(fd, response);
		return ;
	}
	//si le client qui envoie l'invitation n'est pas membre du channel
	if (!_channels[channel_target].hasMember(_clients[fd]))
	{
		response = ":" + this->getHostname() + " " + ERR_NOTONCHANNEL + " " + nick_target + " #" + channel_target + " :You're not on that channel";
		writeMessage(fd, response);
		return ;
	}
	if (_channels[channel_target].isOperator(_clients[fd].getNickname()))
	{
		_channels[channel_target].addGuest(nick_target);
		//si le client invite est deja membre du channel
		if (_channels[channel_target].hasMember(*getClientByNickname(nick_target)))
		{
			response = ":" + this->getHostname() + " " + ERR_USERONCHANNEL + " " + nick_target + " #" + channel_target + " :is already on channel";
			writeMessage(fd, response);
			return ;
		}
		response = ":" + _clients[fd].getNickname() + " INVITE " + nick_target + " #" + channel_target;
		writeMessage(getClientByNickname(nick_target)->getFd(), response);

	}
	//si le client n'a pas les droits pour envoyer une invitation
	else
	{
		response = ":" + this->getHostname() + " " + ERR_CHANOPRIVSNEEDED + " " + nick_target + " #" + channel_target + " :You're not channel operator";
		writeMessage(fd, response);
		return ;
	}
}

void	IrcServer::command_kick(std::vector<std::string> &command, int fd)
{
	std::string response;
	std::string	channel_target = command[1].substr(1, std::string::npos);
	std::string	nick_target = command[2];

	//si il n'y a pas le bon nombre de parametres
	if (command.size() < 3)
	{
		response = ":" + this->getHostname() + " " + ERR_NEEDMOREPARAMS + " " + _clients[fd].getNickname() + " INVITE: Not enough parameters";
		writeMessage(fd, response);
		return ;
	}
	//si le nickname de la personne invitee n'existe pas 
	if (!getClientByNickname(nick_target))
	{
		response = ":" + this->getHostname() + " " + ERR_NOSUCHNICK + " " + nick_target + " :No such nick";	
		writeMessage(fd, response);
		return ;
	}
	//si le nom du channel specifie n'existe pas
	if (!_channels.count(channel_target))
	{
		response = ":" + this->getHostname() + " " + ERR_NOSUCHCHANNEL + " " + nick_target + " #" + channel_target + " :No such channel";
		writeMessage(fd, response);
		return ;
	}
	//si le client qui envoie l'invitation n'est pas membre du channel
	if (!_channels[channel_target].hasMember(_clients[fd]))
	{
		response = ":" + this->getHostname() + " " + ERR_NOTONCHANNEL + " " + nick_target + " #" + channel_target + " :You're not on that channel";
		writeMessage(fd, response);
		return ;
	}
	//si le client est operateur
	if (_channels[channel_target].isOperator(_clients[fd].getNickname()))
	{
		response = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getUsername() + "@"
			+ _clients[fd].getIPaddress() + " KICK #" + channel_target + " " + nick_target;
		if (command.size() > 3)
		{
			for (size_t i = 3; i < command.size(); i++)
			{
				response += " ";
				response += command[i];
			}
		}
		std::map<std::string, Client>::iterator user_it;
		for(user_it = _channels[channel_target].getIterator(); user_it != _channels[channel_target].getIterator_end(); ++user_it)
			writeMessage(user_it->second.getFd(), response);
		_channels[channel_target].removeMember(*getClientByNickname(nick_target));
	}
	//si le client n'est pas operateur
	else
	{
		response = ":" + this->getHostname() + " " + ERR_CHANOPRIVSNEEDED + " " + nick_target + " #" + channel_target + " :You're not channel operator";
		writeMessage(fd, response);
		return ;
	}
}

//getters

Client	*IrcServer::getClientByUsername(std::string &username)
{
	std::map<int, Client>::iterator it = _clients.begin();

	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (!username.compare(it->second.getUsername()))
			return (&it->second);
	}
	return (0);
}

Client	*IrcServer::getClientByNickname(std::string &nickname)
{
	std::map<int, Client>::iterator it;

	for (it = _clients.begin(); it != _clients.end(); it++)
	{
		if (!nickname.compare(it->second.getNickname()))
			return (&it->second);
	}
	return (0);
}
