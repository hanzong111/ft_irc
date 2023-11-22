#include "TCPServer.hpp"
#include "TCPClientConn.hpp"
#include "TCPHost.hpp"
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <string>
#include <cerrno>
#include <iostream>
#include <vector>
#include <arpa/inet.h>

TCPServer::TCPServer(const std::string &ip_addr, uint16_t port_num) :
	TCPHost(),
	n_events(0)
{
	pollfd_struct = NULL;
	createTCPSocket();
	bindToInterface(ip_addr, port_num);
}

TCPServer::TCPServer(const TCPServer &other) :
	TCPHost(),
	n_events(0)
{
	pollfd_struct = NULL;
	*this = other;
}

TCPServer &TCPServer::operator=(const TCPServer &other)
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
	memcpy(pollfd_struct, other.pollfd_struct, sizeof(*pollfd_struct));
	n_events = other.n_events;
	return (*this);
}

TCPServer::~TCPServer()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
}

int	TCPServer::createTCPSocket()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd == -1)
		throw std::runtime_error("socket: " + std::string(std::strerror(errno)));
	return (fd);
}

int	TCPServer::bindToInterface(const std::string &ip_addr, uint16_t port_num)
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

int	TCPServer::startListening()
{
	if (fd == -1)
		throw std::range_error("startListening: Invalid fd");
	else if (listen(fd, SOMAXCONN) == -1)
		throw std::runtime_error("listen: " + std::string(std::strerror(errno)));
	pollfd tmp;
	tmp.fd = fd;
	tmp.events = POLLIN;
	tmp.revents = 0;
	pollfd_vect.push_back(tmp);
	pollfd_struct = &pollfd_vect.back();
	return (0);
}

void	TCPServer::acceptConnReq()
{
	try
	{
		TCPClientConn	client(*this);

		addClient(client);
		std::cout << "Client (" << clients.back().getIPAddrStr() << ":"
			<< clients.back().getPortNumH() << ") connected" << std::endl;
		std::cout << "Client fd: " << clients.back().getFd() << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void	TCPServer::addClient(TCPClientConn &client)
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

void	TCPServer::pollEvents(int timeout)
{
	n_events = poll(pollfd_vect.data(), pollfd_vect.size(), timeout);
	if (n_events == -1)
		throw std::runtime_error("poll: " + std::string(std::strerror(errno)));
}

void	TCPServer::updatePollfdPtrs()
{
	pollfd_struct = &pollfd_vect.front();
	for (size_t i = 0; i < clients.size(); i++)
		clients[i].setPollFdPtr(&pollfd_vect[i + 1]);
}

void	TCPServer::updatePollfdStruct()
{
	pollfd_struct[0].fd = fd;
	for (size_t i = 0; i < clients.size(); i++)
		pollfd_struct[i + 1].fd = clients[i].getFd();
}
