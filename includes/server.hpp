#ifndef SERVER_HPP
# define SERVER_HPP

#include "../headers/Macros.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <poll.h>
#include <vector>
#include <ostream>
#include <map>
#include <iterator>
#include <sstream>
#include <signal.h>

#include "channel.hpp"


class	IrcServer
{
public :

	IrcServer(); 
	~IrcServer();


	// void delete_fds();
	// void sig_handler(int sig);
	void newClient();
	void startServer();
	void newCommand(int fd, int i);
	void parsing(std::string &command, int fd);
	void sortingWords(std::string &word, int fd);
	void parsingCommand(std::vector<std::string> &command, int fd);
	void disconnectClient(int fd, std::map<int, Client> &_clients, std::map<std::string, Channel> &_channels);

	//commands

	void command_pong(int fd);
	void command_pass(std::string &mdp, int fd);
	void command_nick(std::string &nick, int fd);
	int	 command_user(std::vector<std::string> &command, int fd);
	void command_prvmsg(std::vector<std::string> &arg, int fd);
	void command_join(std::vector<std::string> &arg, int fd);
	void command_part(std::vector<std::string> &command, int fd);
	void command_who(std::vector<std::string> &command, int fd);
	void command_mode(std::vector<std::string> &command, int fd);
	void command_topic(std::vector<std::string> &arg, int fd);
	void command_invite(std::vector<std::string> &command, int fd);
	void command_kick(std::vector<std::string> &command, int fd);
	
	//mode sub command
	
	void command_modei(char sign, int fd, std::vector<std::string> &command, std::string chan_name);
	void command_modet(char sign, int fd, std::vector<std::string> &command, std::string chan_name);
	void command_modek(char sign, int fd, std::vector<std::string> &command, std::string chan_name);
	void command_modeo(char sign, int fd, std::vector<std::string> &command, std::string chan_name);
	void command_model(char sign, int fd, std::vector<std::string> &command, std::string chan_name);

	//setters

	void setPort(unsigned int port) {this->_port = port;};
	void setSockfd(int sock_fd) {this->_sock_fd = sock_fd;};
	void setPassword(std::string password) {this->_password = password;};
	void setNfds(unsigned int nfds) {this->_nfds = nfds;};
	void setHostname(std::string hostname) {this->_hostname = hostname;};

	// getters

	Client			*getClientByUsername(std::string &username);
	Client			*getClientByNickname(std::string &nickname);
	unsigned int	getPort() {return (this->_port);};
	int				getSockfd() {return (this->_sock_fd);};
	std::string		getPassword() {return (this->_password);};
	std::string		getHostname() {return (this->_hostname);};

	std::vector<struct pollfd> fds;
	static bool flag;

private:

	std::string		_hostname;
	unsigned int	_port;
	std::string		_password;
	unsigned int	_sock_fd;
	std::map<int, Client>	_clients;
	std::map<std::string, Channel>	_channels;
	unsigned int	_nfds;
};

int	writeMessage(int socket_fd, std::string &message);
std::vector<std::string> split(const std::string &str);
void	sig_handler(int sig);

#endif
