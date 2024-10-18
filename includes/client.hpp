#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <poll.h>
#include <vector>
#include <ostream>
#include <map>

class Client
{
public :

	Client();
	~Client();

	unsigned int	_is_valid;

	int	isRegistered();

	//setters
	void setFd(int fd);
	void setRealname(std::string &Realname);
	void setUsername(std::string &Username);
	void setNickname(std::string &Nickname);
	void setBool(int code);
	void setHostname(std::string &hostname);
	void setIPaddress(char *IPaddress);
	void setIsRegistered();

	//getters

	int	getBools();
	int	getFd();
	std::string		getRealname();
	std::string		getUsername();
	std::string		getNickname();
	std::string		getHostname();
	std::string&	getBuffer();
	char*			getIPaddress();

private :

	int				_fd;
	//booleen pour checker que le PASS a deja ete entre avant NICK et USER
	//booleens pour checker si les trois commandes sont entrees
	bool			_pass_bool;
	bool			_Username_bool;
	bool			_Nickname_bool;
	bool			_registered;
	bool			_operator;
	std::string		_Realname;
	std::string		_Username;
	std::string		_Nickname;
	std::string		_buffer;
	char			*_IPaddress;
};

#endif
