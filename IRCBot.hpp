#ifndef FT_IRC_IRCBOT_HPP
# define FT_IRC_IRCBOT_HPP

# include "TCPConn.hpp"
# include <poll.h>

class IRCBot : public TCPConn
{
	public:
		IRCBot(const std::string &server_ip_addr, uint16_t server_port_num);
		IRCBot(const IRCBot &other);
		IRCBot &operator=(const IRCBot &other);
		virtual ~IRCBot();
		
		void	authenticateConnection(const std::string &password);
		bool	registerAsUser(const std::string &nickname, const std::string &username,
					const std::string &realname);
		bool	parrotPRIVMSG();

		std::string	nickname;
		std::string	username;
		std::string	realname;
	
	private:
		pollfd	pollfd_struct_obj;
};

#endif
