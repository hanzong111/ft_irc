#ifndef FT_IRC_TCPSERVER_IMPL_HPP
# define FT_IRC_TCPSERVER_IMPL_HPP

#include "TCPServer.hpp"
#include "TCPConn.hpp"
#include "TCPHost.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <string>
#include <cerrno>
#include <iostream>
#include <vector>
#include <arpa/inet.h>

template <typename ClientType>
TCPServer<ClientType>::TCPServer(const std::string &ip_addr, uint16_t port_num) :
	TCPHost(),
	n_events(0)
{
	pollfd tmp;
	
	createTCPSocket();
	tmp.fd = fd;
	tmp.events = POLLIN;
	tmp.revents = 0;
	pollfd_vect.push_back(tmp);
	pollfd_struct = &pollfd_vect.front();
	bindToInterface(ip_addr, port_num);
}

template <typename ClientType>
TCPServer<ClientType>::TCPServer(const TCPServer<ClientType> &other) :
	TCPHost(),
	n_events(0)
{
	pollfd	tmp;

	tmp.fd = -1;
	tmp.events = POLLIN;
	tmp.revents = 0;
	pollfd_vect.push_back(tmp);
	// pollfd_vect was empty, so `tmp` must be at the front.
	pollfd_struct = &pollfd_vect.front();
	*this = other;
}

template <typename ClientType>
TCPServer<ClientType> &TCPServer<ClientType>::operator=(const TCPServer<ClientType> &other)
{
	if (this == &other)
		return (*this);
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
	fd = dup(other.fd);
	if (fd == -1)
		throw std::runtime_error(std::string("dup: ") + strerror(errno));
	addr = other.addr;
	addr_len = other.addr_len;
	pollfd_vect = other.pollfd_vect;
	clients = other.clients;
	if (pollfd_struct != NULL && pollfd_struct != other.pollfd_struct)
	{
		memcpy(pollfd_struct, other.pollfd_struct, sizeof(*pollfd_struct));
		pollfd_struct->fd = fd;
	}
	n_events = other.n_events;
	return (*this);
}

template <typename ClientType>
TCPServer<ClientType>::~TCPServer()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
}

template <typename ClientType>
int	TCPServer<ClientType>::createTCPSocket()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	if (fd == -1)
		throw std::runtime_error("socket: " + std::string(std::strerror(errno)));
	return (fd);
}

template <typename ClientType>
int	TCPServer<ClientType>::bindToInterface(const std::string &ip_addr, uint16_t port_num)
{
	if (fd == -1 && createTCPSocket() == -1)
		return (-1);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	if (addr.sin_addr.s_addr == (in_addr_t)(-1))
		throw std::range_error("bindToInterface: Invalid address specified");
	addr.sin_port = htons(port_num);
	if (bind(fd, (sockaddr *) &addr, sizeof(addr)) == -1)
		throw std::runtime_error("bind: " + std::string(std::strerror(errno)));
	return (0);
}

template <typename ClientType>
int	TCPServer<ClientType>::startListening()
{
	if (fd == -1)
		throw std::range_error("startListening: Invalid fd");
	else if (listen(fd, SOMAXCONN) == -1)
		throw std::runtime_error("listen: " + std::string(std::strerror(errno)));
	return (0);
}

template <typename ClientType>
void	TCPServer<ClientType>::acceptConnReq()
{
	try
	{
		ClientType	client(*this);

		addClient(client);
		std::cout << "Client (" << clients.back().getIPAddrStr() << ":"
			<< clients.back().getPortNumH() << ") connected" << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

template <typename ClientType>
void	TCPServer<ClientType>::addClient(ClientType &client)
{
	pollfd	tmp;
	bool need_update_pollfd_ptr = false;
	bool need_update_pollfd_struct = false;

	tmp.fd = client.getFd();
	tmp.events = POLLIN;
	tmp.revents = 0;

	if (pollfd_vect.size() == pollfd_vect.capacity())
		need_update_pollfd_ptr = true;
	pollfd_vect.push_back(tmp);
	if (clients.size() == clients.capacity())
		need_update_pollfd_struct = true;
	clients.push_back(client);
	pollfd_vect.back().fd = clients.back().getFd();
	clients.back().setPollFdPtr(&pollfd_vect.back());
	if (need_update_pollfd_ptr)
		updatePollfdPtrs();
	if (need_update_pollfd_struct)
		updatePollfdStruct();
}

template <typename ClientType>
void	TCPServer<ClientType>::removeClient(ClientType &client)
{
	for (typename std::vector<ClientType>::iterator it = clients.begin(); it < clients.end(); it++)
	{
		if (&(*it) == &client)
		{
			clients.erase(it);
			pollfd_vect.erase(pollfd_vect.begin());
			updatePollfdStruct();
			break ;
		}
	}
}

template <typename ClientType>
void	TCPServer<ClientType>::pollEvents(int timeout)
{
	n_events = poll(pollfd_vect.data(), pollfd_vect.size(), timeout);
	if (n_events == -1)
		throw std::runtime_error("poll: " + std::string(std::strerror(errno)));
}

template <typename ClientType>
void	TCPServer<ClientType>::updatePollfdPtrs()
{
	pollfd_struct = &pollfd_vect.front();
	for (size_t i = 0; i < clients.size(); i++)
		clients[i].setPollFdPtr(&pollfd_vect[i + 1]);
}

template <typename ClientType>
void	TCPServer<ClientType>::updatePollfdStruct()
{
	pollfd_struct[0].fd = fd;
	for (size_t i = 0; i < clients.size(); i++)
		pollfd_struct[i + 1].fd = clients[i].getFd();
}

#endif
