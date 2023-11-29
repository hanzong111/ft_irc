#ifndef FT_IRC_TCPSERVER_HPP
# define FT_IRC_TCPSERVER_HPP

# include "TCPHost.hpp"
# include <poll.h>
# include <netinet/in.h>
# include <string>
# include <vector>

template <typename ClientType>
class TCPServer : virtual public TCPHost
{
	public:
		TCPServer<ClientType> &operator=(const TCPServer<ClientType> &other);
		virtual ~TCPServer();

		int				startListening();
		void			acceptConnReq();
		void			addClient(ClientType &client);
		void			removeClient(ClientType &client);
		void			pollEvents(int timeout);
		virtual void	handleEvents() = 0;

	protected:
		TCPServer(const std::string &ip_addr, uint16_t port_num);
		TCPServer(const TCPServer<ClientType> &other);

		int		createTCPSocket();
		int		bindToInterface(const std::string &ip_addr, uint16_t port_num);
		void	updatePollfdPtrs();
		void	updatePollfdStruct();
		
		// `pollfd_vect.front()` is always contains the pollfd struct for the
		// server's listening port. Structs for clients start at offset of 1
		// from `pollfd_vect.front()`.
		std::vector<pollfd>		pollfd_vect;
		// `clients` should always be in the same order as the pollfd structs
		// in `pollfd_vect`. 
		std::vector<ClientType>	clients;
		pollfd					*pollfd_struct; // Always points to `pollfd_vect.front()`
		int						n_events; // Number of polled events to handle
};

#endif
