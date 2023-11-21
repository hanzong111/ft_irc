#ifndef FT_IRC_TESTSERVER_HPP
# define FT_IRC_TESTSERVER_HPP

# include "TCPServer.hpp"
# include <poll.h>
# include <netinet/in.h>
# include <string>
# include <vector>

class TestServer : virtual public TCPServer
{
	public:
		TestServer(const std::string &ip_addr, uint16_t port_num);
		TestServer(const TestServer &other);
		TestServer &operator=(const TestServer &other);
		virtual ~TestServer();

		virtual void	handleEvents();
		void			startServer();
	
	private:
		size_t	handlePollIn(size_t ind);
		size_t	handlePollOut(size_t ind);
};

#endif
