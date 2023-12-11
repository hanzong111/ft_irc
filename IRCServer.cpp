#include "IRCServer.hpp"
#include "TCPServer.hpp"
#include "TCPServer_impl.hpp"
#include "IRCUser.hpp"
#include "IRCMessage.hpp"
#include "IRCNumericReplies.hpp"
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <map>
#include <ctime>
#include <iostream>

//std::map<std::string, IRCServer::MemFuncPtr> serv_func_map;

IRCServer::IRCServer(const std::string &ip_addr, uint16_t port_num) :
	TCPHost(),
	TCPServer(ip_addr, port_num),
	servername(ip_addr),
	time_created(IRCServer::getCurerntTimeAsStr())
{
	if (serv_func_map.empty())
		populateServFuncMap();
	if (chan_func_map.empty())
		populateChanFuncMap();
}

IRCServer::IRCServer(const IRCServer &other) :
	TCPHost(),
	TCPServer(other)
{
	if (serv_func_map.empty())
		populateServFuncMap();
	if (chan_func_map.empty())
		populateChanFuncMap();
	*this = other;
}

IRCServer &IRCServer::operator=(const IRCServer &other)
{
	if (this == &other)
		return (*this);
	TCPServer::operator=(other);
	if (serv_func_map.empty())
		populateServFuncMap();
	if (chan_func_map.empty())
		populateChanFuncMap();
	conn_pass = other.conn_pass;
	servername = other.servername;
	channels = other.channels;
	time_created = other.time_created;
	updateUsersMap();
	return (*this);
}

IRCServer::~IRCServer()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
}

void	IRCServer::setConnPass(const std::string &pass)
{
	conn_pass = pass;
}

void	IRCServer::handleEvents()
{
	int	count = 0;
	if (pollfd_struct->revents) // Listening socket events, connection request
	{
		std::cout << "Connection request received" << std::endl;
		count++;
		if (pollfd_struct->revents & POLLIN)
			acceptConnReq();
	}
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].getPollREvents() & POLLIN)
		{
			count++;
			i = handlePollIn(i);
		}
		else if (clients[i].getPollREvents() & POLLOUT)
		{
			count++;
			handlePollOut(i);
		}
		if (count >= n_events)
			break ;
	}
}

void	IRCServer::startServer()
{
	startListening();
	std::cout << "Server is now listening for requests on " << getIPAddrStr()
			<< ":" << getPortNumH() << std::endl;
	while (true)
	{
		std::cout << "Polling for events" << std::endl;
		pollEvents(-1);
		std::cout << "Handling events" << std::endl;
		handleEvents();
	}
}

size_t	IRCServer::handlePollIn(size_t ind)
{
	ssize_t	n_bytes_recv;

	std::cout << "ClientPollInEvent" << std::endl;
	n_bytes_recv = clients[ind].requestRecv("\r\n", 2, 0);
	if (n_bytes_recv > 0) // Complete recv
	{
		do {
			char	buf[n_bytes_recv + 1];
			clients[ind].retrieveRecvBuf(buf, NULL);
			buf[n_bytes_recv] = '\0';

			processCommands(clients[ind], std::string(buf));
			n_bytes_recv = clients[ind].checkRecvBuf("\r\n", 2);
		} while (n_bytes_recv > 0);
	}
	else if (n_bytes_recv == -1) // Disconnection
	{
		std::cout << "Client (" << clients[ind].getIPAddrStr() << ":"
			<< clients[ind].getPortNumH() << ") disconnected" << std::endl;
		users_map.erase(clients[ind].getNickname());
		removeClient(clients[ind]);
		updateUsersMap();
		return (ind - 1);
	}
	return (ind);
}

size_t	IRCServer::handlePollOut(size_t ind)
{
	ssize_t	n_bytes_send;

	std::cout << "ClientPollOutEvent" << std::endl;
	n_bytes_send = clients[ind].processSendQueue(0);
	return (n_bytes_send);
}

void	print_client_cmd(IRCMessage &msg)
{
	static int	count = 0;

	count++;
	std::cout << RED << count << std::endl;
	std::cout << GREEN;
	std::cout << "prefix :" + msg.prefix << std::endl;
	std::cout << BLUE;
	std::cout << "command :" + msg.command << std::endl;
	std::cout << YELLOW;
	int	i = 0;
	for (std::vector<std::string>::iterator it = msg.params.begin(); it != msg.params.end(); ++it) {
		i++;
        std::cout << "arg[" << i << "] : " << *it << "\n";
    }
	std::cout << DEF_COLOR;
}

void	IRCServer::processCommands(IRCUser &user, const std::string &cmd)
{
	// Parse message
	IRCMessage	msg(cmd);
	print_client_cmd(msg);
	// Check authentication status
	if (!conn_pass.empty() && !user.isAuthenticated() && msg.command != "PASS")
		return ;
	// Retrieve function pointer

	if(msg.for_Channel() == true && msg.command != "JOIN")
		Channel_commands(user, msg);
	else
		Server_commands(user, msg);
}

void	IRCServer::Server_commands(IRCUser &user,const IRCMessage &msg)
{
	std::cout << RED << "inside Server Commands" << DEF_COLOR << std::endl;
	MemFuncPtr	f;

	try
	{
		f = serv_func_map.at(msg.command);
	}
	catch (std::out_of_range &e)
	{
		std::cerr << "Warning: Invalid command (" << msg.command << ")" << std::endl;
		return ;
	}
	(this->*f)(user, msg);
}

void	IRCServer::updateUsersMap()
{
	size_t	ind = 0;
	bool	is_resized = (clients.size() != users_map.size());

	if (is_resized)
		users_map.clear();

	for (std::vector<IRCUser>::const_iterator it = clients.begin(); it < clients.end(); it++)
		users_map[it->getNickname()] = ind++;
}

void	IRCServer::populateServFuncMap()
{
	serv_func_map["PASS"] = &IRCServer::S_handlePASS;
	serv_func_map["NICK"] = &IRCServer::S_handleNICK;
	serv_func_map["USER"] = &IRCServer::S_handleUSER;
	serv_func_map["OPER"] = &IRCServer::S_handleOPER;
	serv_func_map["MODE"] = &IRCServer::S_handleMODE;
	serv_func_map["QUIT"] = &IRCServer::S_handleQUIT;
	serv_func_map["JOIN"] = &IRCServer::S_handleJOIN;
}

void	IRCServer::populateChanFuncMap()
{
	chan_func_map["WHO"] = &IRCServer::C_handleWHO;
}

void	IRCServer::C_handleWHO(IRCUser &user, const IRCMessage &msg)
{
	(void)msg;
	std::string reply = RPL_NAMREPLY(servername, user.getNickname(), "=", "Try", "Hanz Henr Zoe");
	user.queueSend(reply.c_str(), reply.size());
}

std::string	IRCServer::getCurerntTimeAsStr()
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timeinfo);
	return (buffer);
}

bool	IRCServer::isChanneltaken(std::string &channelname)
{
	if(!channels.empty())
		return(channels.find(channelname) != channels.end());
	else
		return(false);
}
