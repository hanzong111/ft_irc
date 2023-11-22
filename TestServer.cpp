#include "TestServer.hpp"
#include "TCPServer.hpp"
#include "TCPClientConn.hpp"
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <cstring>
#include <string>
#include <cerrno>
#include <iostream>

TestServer::TestServer(const std::string &ip_addr, uint16_t port_num) :
	TCPHost(),
	TCPServer(ip_addr, port_num)
{}

TestServer::TestServer(const TestServer &other) :
	TCPHost(),
	TCPServer(other)
{}

TestServer &TestServer::operator=(const TestServer &other)
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
	return (*this);
}

TestServer::~TestServer()
{
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
}

void	TestServer::handleEvents()
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

void	TestServer::startServer()
{
	startListening();
	std::cout << "Server is now listening" << std::endl;
	while (true)
	{
		std::cout << "Polling for events" << std::endl;
		pollEvents(-1);
		std::cout << "Handling events" << std::endl;
		handleEvents();
	}
}

size_t	TestServer::handlePollIn(size_t ind)
{
	ssize_t	n_bytes_recv;

	std::cout << "ClientPollInEvent" << std::endl;
	n_bytes_recv = clients[ind].requestRecv("\r\n", 2, 0);
	if (n_bytes_recv > 0) // Complete recv
	{
		char	buf[n_bytes_recv];
		clients[ind].retrieveRecvBuf(buf, NULL);
		for (std::vector<TCPClientConn>::iterator it = clients.begin(); it < clients.end(); it++)
		{
			if (it != clients.begin() + ind)
				it->queueSend(buf, n_bytes_recv);
		}
	}
	else if (n_bytes_recv == -1) // Disconnection
	{
		std::cout << "Client (" << clients[ind].getIPAddrStr() << ":"
			<< clients[ind].getPortNumH() << ") disconnected" << std::endl;
		// clients.erase(clients.begin() + ind);
		// pollfd_vect.erase(pollfd_vect.begin() + 1 + ind);
		// updatePollfdStruct();
		removeClient(clients[ind]);
		return (ind - 1);
	}
	return (ind);
}

size_t	TestServer::handlePollOut(size_t ind)
{
	ssize_t	n_bytes_send;

	std::cout << "ClientPollOutEvent" << std::endl;
	n_bytes_send = clients[ind].processSendQueue(0);
	return (n_bytes_send);
}
