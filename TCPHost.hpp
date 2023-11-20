#ifndef FT_IRC_TCPHOST_HPP
# define FT_IRC_TCPHOST_HPP

# include <string>
# include <netinet/in.h>

class TCPHost
{
	public:
		TCPHost();
		TCPHost(const std::string &ip_addr, uint16_t port_num);
		TCPHost(const TCPHost &other);
		TCPHost &operator=(const TCPHost &other);
		virtual ~TCPHost();

		int			getFd() const throw();
		uint16_t	getPortNumN() const throw();
		uint16_t	getPortNumH() const throw();
		in_addr_t	getIPAddr() const throw();
		std::string	getIPAddrStr() const throw();

	protected:
		int			fd;
		sockaddr_in	addr;
		socklen_t	addr_len;
};

#endif
