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
	time_created(IRCServer::getCurrentTimeAsStr()),
	shutdown(false)
{
	if (serv_func_map.empty())
		populateServFuncMap();
}

IRCServer::IRCServer(const IRCServer &other) :
	TCPHost(),
	TCPServer(other)
{
	if (serv_func_map.empty())
		populateServFuncMap();
	*this = other;
}

IRCServer &IRCServer::operator=(const IRCServer &other)
{
	if (this == &other)
		return (*this);
	TCPServer::operator=(other);
	if (serv_func_map.empty())
		populateServFuncMap();
	conn_pass = other.conn_pass;
	servername = other.servername;
	channels = other.channels;
	time_created = other.time_created;
	shutdown = other.shutdown;
	updateUsersMap();
	return (*this);
}

IRCServer::~IRCServer()
{}

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
		{
			acceptConnReq();
			updateUsersMap();
		}
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
	std::string reply;

	startListening();
	std::cout << "Server is now listening for requests on " << getIPAddrStr()
			<< ":" << getPortNumH() << std::endl;
	while (!shutdown)
	{
		std::cout << "Polling for events" << std::endl;
		pollEvents(-1);
		std::cout << "Handling events" << std::endl;
		handleEvents();
	}
}

void	IRCServer::createChannel( IRCUser &user, const std::string &channel_name)
{
	std::map<std::string, IRCChannel>::iterator	it;

	channels.insert(std::pair<std::string, IRCChannel>(channel_name, IRCChannel(channel_name)));
	it = channels.find(channel_name);
	if(it == channels.end())
		std::cout << channel_name + " :No channel found!\n";
	else
	{
		it->second.addOper(user.getNickname());
		it->second.setCreator(user.getNickname());
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
			if (std::string(buf).substr(0, 4) == "QUIT")
				return (ind - !(ind == 0));
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
		return (ind - !(ind == 0));
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

void	IRCServer::broadcastToUsers(const UsersList &target_nicknames,
			const std::string &message)
{
	size_t	target_ind;
	
	for (UsersList::const_iterator it = target_nicknames.begin(); it != target_nicknames.end(); it++)
	{
		try
		{
			target_ind = users_map.at(*it);
			clients[target_ind].queueSend(message.c_str(), message.size());
		}
		catch (const std::out_of_range &e)
		{
			std::cerr << "Warning: Nickname not found (" << *it << ")" << std::endl;
		}
	}
}

int	IRCServer::broadcastToChannel(const std::string &channel_name, const std::string &message)
{
	IRCChannel	*target_channel = NULL;

	try
	{
		target_channel = &channels.at(channel_name);
	}
	catch (const std::out_of_range &e)
	{
		std::cerr << "Warning: Channel not found (" << channel_name << ")" << std::endl;
		return (-1);
	}
	broadcastToUsers(target_channel->getUsers(), message);
	return (0);
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
	serv_func_map["PRIVMSG"] = &IRCServer::S_handlePRIVMSG;
	serv_func_map["NOTICE"] = &IRCServer::S_handleNOTICE;
	serv_func_map["DIE"] = &IRCServer::S_handleDIE;
	serv_func_map["WHO"] = &IRCServer::C_handleWHO;
	serv_func_map["PART"] = &IRCServer::C_handlePART;
	serv_func_map["TOPIC"] = &IRCServer::C_handleTOPIC;
	serv_func_map["INVITE"] = &IRCServer::C_handleINVITE;
	serv_func_map["LIST"] = &IRCServer::C_handleLIST;
	serv_func_map["KICK"] = &IRCServer::C_handleKICK;
	serv_func_map["NAMES"] = &IRCServer::C_handleNAMES;
}

std::string	IRCServer::getCurrentTimeAsStr()
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
