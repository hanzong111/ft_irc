#include "TCPHost.hpp"
#include <unistd.h>
#include <cstring>
#include <string>
#include <cerrno>
#include <iostream>
#include <arpa/inet.h>

TCPHost::TCPHost() :
	fd(-1),
	addr_len(sizeof(addr))
{
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
}

TCPHost::TCPHost(const std::string &ip_addr, uint16_t port_num) :
	fd(-1)
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
	addr.sin_port = htons(port_num);
}

TCPHost::TCPHost(const TCPHost &other) :
	fd(-1),
	addr(other.addr),
	addr_len(other.addr_len)
{
	if (other.fd == -1)
		return ;
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
	fd = dup(other.fd);
	if (fd == -1)
		throw std::runtime_error(std::string("dup: ") + std::strerror(errno));
}

TCPHost &TCPHost::operator=(const TCPHost &other)
{
	if (this == &other)
		return (*this);
	if (fd != -1 && close(fd) == -1)
		std::cerr << "Warning: Failed to close socket" << std::endl;
	fd = dup(other.fd);
	if (fd == -1)
		throw std::runtime_error(std::string("dup: ") + std::strerror(errno));
	fd = other.fd;
	addr = other.addr;
	addr_len = other.addr_len;
	return (*this);
}

TCPHost::~TCPHost()
{}

int	TCPHost::getFd() const throw()
{
	return (fd);
}

uint16_t	TCPHost::getPortNumN() const throw()
{
	return (addr.sin_port);
}

uint16_t	TCPHost::getPortNumH() const throw()
{
	return (ntohs(addr.sin_port));
}

in_addr_t	TCPHost::getIPAddr() const throw()
{
	return (addr.sin_addr.s_addr);
}

std::string	TCPHost::getIPAddrStr() const throw()
{
	return (inet_ntoa(addr.sin_addr));
}
