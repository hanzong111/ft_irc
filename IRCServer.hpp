#ifndef FT_IRC_IRCSERVER_HPP
# define FT_IRC_IRCSERVER_HPP

# include "TCPServer.hpp"
# include "IRCChannel.hpp"
# include <poll.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <map>
# include <set>

# define IRCSERVER_VER "AmnesiacIRC-v0.1"
# define IRCSERVER_SUPPORTED_USER_MODES "iroO"
# define IRCSERVER_SUPPORTED_CHANNEL_MODES "Oov"

class IRCMessage;
class IRCUser;

class IRCServer : virtual public TCPServer<IRCUser>
{
	public:
		typedef void (IRCServer::*MemFuncPtr)(IRCUser &, const IRCMessage &);

		IRCServer(const std::string &ip_addr, uint16_t port_num);
		IRCServer(const IRCServer &other);
		IRCServer &operator=(const IRCServer &other);
		virtual ~IRCServer();

		void			setConnPass(const std::string &pass);
		virtual void	handleEvents();
		void			startServer();
	
	private:
		static std::string getCurerntTimeAsStr();

		size_t	handlePollIn(size_t ind);
		size_t	handlePollOut(size_t ind);
		void	processCommands(IRCUser &user,const std::string &cmd);
		void	populateFuncMap();
		bool	addUser();
		void	updateUsersMap();
		void	sendWelcomeMessages(IRCUser &user);
		void	handlePASS(IRCUser &user, const IRCMessage &msg);
		void	handleNICK(IRCUser &user, const IRCMessage &msg);
		void	handleUSER(IRCUser &user, const IRCMessage &msg);
		void	handleOPER(IRCUser &user, const IRCMessage &msg);
		void	handleMODE(IRCUser &user, const IRCMessage &msg);

		std::map<std::string, MemFuncPtr>			func_map;
		// `users_map` maps username to index in `clients`
		std::map<std::string, size_t>				users_map;
		std::map<std::string, IRCChannel>			channels;
		std::string									conn_pass;
		std::string									servername;
		std::string									time_created;
};

#endif
