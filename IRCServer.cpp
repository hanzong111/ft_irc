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

#define OPERATOR_PASS "1234"

//std::map<std::string, IRCServer::MemFuncPtr> func_map;

IRCServer::IRCServer(const std::string &ip_addr, uint16_t port_num) :
	TCPHost(),
	TCPServer(ip_addr, port_num),
	servername(ip_addr),
	time_created(IRCServer::getCurerntTimeAsStr())
{
	if (func_map.empty())
		populateFuncMap();
}

IRCServer::IRCServer(const IRCServer &other) :
	TCPHost(),
	TCPServer(other)
{
	if (func_map.empty())
		populateFuncMap();
	*this = other;
}

IRCServer &IRCServer::operator=(const IRCServer &other)
{
	if (this == &other)
		return (*this);
	TCPServer::operator=(other);
	if (func_map.empty())
		populateFuncMap();
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

void	IRCServer::processCommands(IRCUser &user, const std::string &cmd)
{
	// Parse message
	IRCMessage	msg(cmd);
	// Check authentication status
	if (!conn_pass.empty() && !user.isAuthenticated() && msg.command != "PASS")
		return ;
	// Retrieve function pointer
	MemFuncPtr	f;
	try
	{
		f = func_map.at(msg.command);
	}
	catch (std::out_of_range &e)
	{
		std::cerr << "Warning: Invalid command (" << msg.command << ")" << std::endl;
		return ;
	}
	// Call function
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

void	IRCServer::sendWelcomeMessages(IRCUser &user)
{
	std::string	str;

	// Send welcome messages
	str = RPL_WELCOME(servername, user.getNickname(),
		user.getUsername(), user.getRealName());
	str += RPL_YOURHOST(servername, user.getNickname(), IRCSERVER_VER);
	str += RPL_CREATED(servername, user.getNickname(), time_created);
	str += RPL_MYINFO(servername, user.getNickname(),
				IRCSERVER_VER,
				IRCSERVER_SUPPORTED_USER_MODES,
				IRCSERVER_SUPPORTED_CHANNEL_MODES);
	user.queueSend(str.c_str(), str.size());
}

void	IRCServer::populateFuncMap()
{
	func_map["PASS"] = &IRCServer::handlePASS;
	func_map["NICK"] = &IRCServer::handleNICK;
	func_map["USER"] = &IRCServer::handleUSER;
	func_map["OPER"] = &IRCServer::handleOPER;
	func_map["MODE"] = &IRCServer::handleMODE;
}

void	IRCServer::handlePASS(IRCUser &user, const IRCMessage &msg)
{
	std::string	reply;

	if (msg.params.size() < 1)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.isAuthenticated() && user.isRegistered())
		reply = ERR_ALREADYREGISTRED(servername, user.getNickname());
	else if (!(msg.params[0] == conn_pass))
		reply = ERR_WRONGPASS(servername, user.getNickname(), msg.command);
	if (!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	else if (msg.params[0] == conn_pass)
		user.makeAuthenticated();
}

void	IRCServer::handleNICK(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 1)
		reply = ERR_NONICKNAMEGIVEN(servername, "*");
	else if (!IRCUser::isValidNickname(msg.params[0]))
		reply = ERR_ERRONEUSNICKNAME(servername, user.getNickname(), msg.params[0]);
	else if (users_map.find(msg.params[0]) != users_map.end())
		reply = ERR_NICKNAMEINUSE(servername, user.getNickname(), msg.params[0]);
	else if (user.getModeFlags() & RESTRICTED)
		reply = ERR_RESTRICTED(servername, user.getNickname());
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}

	// Update IRCUser object
	std::string old_nickname = user.getNickname();
	user.changeNickname(msg.params[0]);
	// Update `users_map`
	size_t ind = users_map[old_nickname];
	users_map.erase(old_nickname);
	users_map[msg.params[0]] = ind;
	// Check if registration is completed
	if (!user.getUsername().empty())
	{
		user.makeRegistered();
		sendWelcomeMessages(user);
	}
}

void	IRCServer::handleUSER(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 4)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.isAuthenticated() && user.getUsername() != "")
		reply = ERR_ALREADYREGISTRED(servername, user.getNickname());
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}
	user.changeUsername(msg.params[0]);
	user.changeRealName(msg.params[3]);
	// It would be a good idea to check if the mode specified in the message
	// is a single digit, but since there is no reply specified for invalid
	// parameter in RFC2812, the check isn't performed here.
	int	mode = std::atoi(msg.params[1].c_str()) & (WALLOPS | INVISIBLE);
	user.setModeFlag(mode);
	// Check if registration is completed
	if (user.getNickname().find(IRCUSER_DEFAULT_NICK_PREFIX) != 0)
	{
		user.makeRegistered();
		sendWelcomeMessages(user);
	}
}

void	IRCServer::handleOPER(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 2)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (msg.params[1] != OPERATOR_PASS)
		reply = ERR_PASSWDMISMATCH(servername, user.getNickname());
	else
	{
		reply = RPL_YOUREOPER(servername, user.getNickname());
		user.setModeFlag(OPER);
	}
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}	
}

void	IRCServer::handleMODE(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;
	std::map<enum IRCUserModes, const char>::iterator flag;

	if (msg.params.size() < 1)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.getNickname() != msg.params[0])
		reply = ERR_USERSDONTMATCH(servername, user.getNickname());
	else if (msg.params.size() == 1)
		reply = RPL_UMODEIS(servername, user.getNickname(), user.getModestr());
	else if (msg.params[1].size() != 2 || !(msg.params[1][0] == '+' || msg.params[1][0] == '-'))
		reply = ERR_UMODEUNKNOWNFLAG(servername, user.getNickname());
	else
	{
		flag = user.getFlag_map().end();
		for (std::map<enum IRCUserModes, const char>::iterator it = user.getFlag_map().begin(); it != user.getFlag_map().end(); ++it)
		{
    		if(it->second == msg.params[1][1])
				flag = it;
    	}
		if(flag == user.getFlag_map().end())
			reply = ERR_UMODEUNKNOWNFLAG(servername, user.getNickname());
		else
		{
			if(msg.params[1][0] == '+')
			{
				if(flag->second == 'o' || flag->second == 'O' || flag->second == 'a')
					return;
				user.setModeFlag(flag->first);
			}
			else if (msg.params[1][0] == '-')
			{
				if(flag->second == 'r')
					return;
				user.clearModeFlag(flag->first);
			}
		}
	}
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}	
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
