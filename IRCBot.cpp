#include "IRCBot.hpp"
#include "IRCMessage.hpp"
#include "charset.h"
#include <poll.h>
#include <iostream>

IRCBot::IRCBot(const std::string &server_ip_addr, uint16_t server_port_num) :
	TCPConn(server_ip_addr, server_port_num)
{
	pollfd_struct_obj.fd = fd;
	pollfd_struct_obj.events = POLLIN;
	pollfd_struct_obj.revents = 0;
	pollfd_struct = &pollfd_struct_obj;
}

IRCBot::IRCBot(const IRCBot &other) :
	TCPHost(),
	TCPConn(other)
{
	*this = other;
}

IRCBot &IRCBot::operator=(const IRCBot &other)
{
	if (this == &other)
		return (*this);
	TCPConn::operator=(other);
	nickname = other.nickname;
	username = other.username;
	realname = other.realname;
	pollfd_struct_obj = other.pollfd_struct_obj;
	pollfd_struct = &pollfd_struct_obj;
	return (*this);
}

IRCBot::~IRCBot()
{}

void	IRCBot::authenticateConnection(const std::string &password)
{
	std::string msg;

	msg += "PASS " + password + "\r\n";
	pollfd_struct->events |= POLLOUT;
	queueSend(msg.c_str(), msg.size());
	while (pollfd_struct->events & POLLOUT)
	{
		poll(pollfd_struct, 1, -1);
		processSendQueue(0);
	}
}

bool	IRCBot::registerAsUser(const std::string &nickname, const std::string &username,
		const std::string &realname)
{
	std::string msg;

	msg += "NICK " + nickname + "\r\n";
	msg += "USER " + username + " 0 * :" + realname + "\r\n";
	pollfd_struct->events |= POLLOUT;
	queueSend(msg.c_str(), msg.size());
	while (pollfd_struct->events & POLLOUT)
	{
		poll(pollfd_struct, 1, -1);
		processSendQueue(0);
	}
	while (checkRecvBuf("\r\n", 2) == 0)
	{
		poll(pollfd_struct, 1, -1);
		if (pollfd_struct->revents & POLLIN && requestRecv("\r\n", 2, 0) == -1)
		{
			std::cerr << "Connection closed by server" << std::endl;
			return (false);
		}
	}
	while (true)
	{
		size_t	msg_len = checkRecvBuf("\r\n", 2);
		char	buf[msg_len + 1];

		retrieveRecvBuf(buf, NULL);
		buf[msg_len] = '\0';
		IRCMessage irc_msg(buf);
		if (irc_msg.numeric_code == "432")
		{
			std::cerr << "Erroneous nickname" << std::endl;
			return (false);
		}
		else if (irc_msg.numeric_code == "433")
		{
			std::cerr << "Nickname is already in use" << std::endl;
			return (false);
		}
		else if (irc_msg.numeric_code == "484")
		{
			std::cerr << "Connection is restricted" << std::endl;
			return (false);
		}
		else if (irc_msg.numeric_code == "004")
		{
			return (true);
		}
		if (checkRecvBuf("\r\n", 2) == 0)
		{
			poll(pollfd_struct, 1, -1);
			if ((pollfd_struct->revents & POLLIN) && requestRecv("\r\n", 2, 0) == -1)
			{
				std::cerr << "Connection closed by server" << std::endl;
				return (false);
			}
		}
	}
}

bool	IRCBot::parrotPRIVMSG()
{
	ssize_t	ret_val = 0;

	while (ret_val == 0 && checkRecvBuf("\r\n", 2) == 0)
	{
		poll(pollfd_struct, 1, -1);
		std::cout << "Data received" << std::endl;
		ret_val = requestRecv("\r\n", 2, 0);
		if (ret_val == -1)
		{
			std::cerr << "Connection closed by server" << std::endl;
			return (false);
		}
	}

	char	buf[ret_val + 1];
	retrieveRecvBuf(buf, NULL);
	buf[ret_val] = '\0';
	
	IRCMessage	irc_msg(buf);
	std::cout << buf << std::endl;
	if (irc_msg.command == "PRIVMSG" && irc_msg.params.size() >= 1)
	{
		std::string	msg;
		std::string target_nickname;

		target_nickname = irc_msg.prefix;
		target_nickname = target_nickname.substr(0, target_nickname.find_first_not_of(CHARSET_ALPHANUMERICS));
		msg = "PRIVMSG " + target_nickname + " :";
		for (size_t i = 1; i < irc_msg.params.size(); i++)
			msg += irc_msg.params[i] + " ";
		msg.erase(msg.end() - 1);
		msg += "\r\n";
		queueSend(msg.c_str(), msg.size());
		while (pollfd_struct->events & POLLOUT)
		{
			poll(pollfd_struct, 1, -1);
			if (pollfd_struct->revents & POLLOUT)
				processSendQueue(0);
		}
	}
	return (true);
}
