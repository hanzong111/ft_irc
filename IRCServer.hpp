#ifndef FT_IRC_IRCSERVER_HPP
# define FT_IRC_IRCSERVER_HPP

# include "TCPServer.hpp"
# include "IRCChannel.hpp"
# include <poll.h>
# include <netinet/in.h>
# include <string>
# include <vector>
# include <iostream>
# include <sstream>
# include <map>
# include <set>

# define IRCSERVER_VER "AmnesiacIRC-v0.1"
# define IRCSERVER_SUPPORTED_USER_MODES "iroO"
# define IRCSERVER_SUPPORTED_CHANNEL_MODES "Oov"

# define DEF_COLOR	 "\033[0;39m"
# define GRAY     	 "\033[0;90m"
# define RED    	 "\033[0;91m"
# define GREEN    	 "\033[0;92m"
# define YELLOW    	 "\033[0;93m"
# define BLUE    	 "\033[0;94m"
# define MAGENTA	 "\033[0;95m"
# define CYAN    	 "\033[0;96m"
# define WHITE    	 "\033[0;97m"

#define SERVER_OPERATOR_PASS "1234"

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
		void	populateServFuncMap();
		void	populateChanFuncMap();
		bool	addUser();
		void	updateUsersMap();
		void	sendWelcomeMessages(IRCUser &user);

		void	Server_commands(IRCUser &user,const IRCMessage &cmd);
		void	Channel_commands(IRCUser &user, const IRCMessage &msg);

		/*	Server Commands		*/
		void	S_handlePASS(IRCUser &user, const IRCMessage &msg);
		void	S_handleNICK(IRCUser &user, const IRCMessage &msg);
		void	S_handleUSER(IRCUser &user, const IRCMessage &msg);
		void	S_handleOPER(IRCUser &user, const IRCMessage &msg);
		void	S_handleMODE(IRCUser &user, const IRCMessage &msg);
		void	S_handleQUIT(IRCUser &user, const IRCMessage &msg);
		void	S_handleJOIN(IRCUser &user, const IRCMessage &msg);
		void	S_handlePRIVMSG(IRCUser &user, const IRCMessage &msg);

		void	create_channel(IRCUser &user, std::map<std::string, std::string>::iterator 	it, std::string *reply);
		void	join_channel(IRCUser &user, std::string &user_key, IRCChannel &channel, std::string *reply);
		bool	isChanneltaken(std::string &channelname);
		/*	Channel Commands	*/
		void	C_handleWHO(IRCUser &user, const IRCMessage &msg);


		std::map<std::string, MemFuncPtr>			serv_func_map;
		std::map<std::string, MemFuncPtr>			chan_func_map;
		// `users_map` maps username to index in `clients`
		std::map<std::string, size_t>				users_map;
		std::map<std::string, IRCChannel>			channels;
		std::string									conn_pass;
		std::string									servername;
		std::string									time_created;
};

#endif
