#ifndef FT_IRC_TCPSERVER_HPP
# define FT_IRC_TCPSERVER_HPP

# include "TCPHost.hpp"
# include <poll.h>
# include <netinet/in.h>
# include <string>
# include <vector>

class TCPClientConn;

class TCPServer : virtual public TCPHost
{
	public:
		TCPServer &operator=(const TCPServer &other);
		virtual ~TCPServer();

		int				startListening();
		void			acceptConnReq();
		void			addClient(TCPClientConn &client);
		void			removeClient(TCPClientConn &client);
		void			pollEvents(int timeout);
		virtual void	handleEvents() = 0;

	protected:
		TCPServer(const std::string &ip_addr, uint16_t port_num);
		TCPServer(const TCPServer &other);

		int		createTCPSocket();
		int		bindToInterface(const std::string &ip_addr, uint16_t port_num);
		void	updatePollfdPtrs();
		void	updatePollfdStruct();
		
		std::vector<pollfd>		pollfd_vect;
		std::vector<TCPClientConn>	clients;
		pollfd					*pollfd_struct;
		int						n_events;
};

#endif
