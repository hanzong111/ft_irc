#ifndef FT_IRC_IRCCHANNEL_HPP
# define FT_IRC_IRCCHANNEL_HPP

# include "IRCMessage.hpp"
# include <string>
# include <set>
# include <map>
# include<iostream>

# define DEF_COLOR	 "\033[0;39m"
# define GRAY     	 "\033[0;90m"
# define RED    	 "\033[0;91m"
# define GREEN    	 "\033[0;92m"
# define YELLOW    	 "\033[0;93m"
# define BLUE    	 "\033[0;94m"
# define MAGENTA	 "\033[0;95m"
# define CYAN    	 "\033[0;96m"
# define WHITE    	 "\033[0;97m"

typedef enum IRCChannelModes
{
	C_KEY = 0x80,
	C_LIMIT = 0x100,
	C_OPER = 0x200,
	C_CREATOR = 0x400,
	C_MODE = 0x800
} e_IRCChannelModes;

typedef std::map<const char, enum IRCChannelModes> IRCChannelModesMap;

class IRCMessage;
class IRCUser;

class IRCChannel
{
	public:
		typedef std::set<std::string> UsersList;
		
		IRCChannel(const std::string &channel_name);
		IRCChannel(const std::string &channel_name, const std::string &channel_key);
		IRCChannel(const IRCChannel &other);
		virtual ~IRCChannel();

		const std::string	&getName() const throw();
		const UsersList		&getUsers() const throw();
		const std::string	&getModestr();
		const std::string	&getKey();
		IRCChannelModesMap 	&getFlag_map();
		void				addUser(const std::string nickname);
		void				removeUser(const std::string nickname);
		bool				isUserInChannel(const std::string nickname) const throw();
		void				muteUser(const std::string nickname);
		bool				isUserMuted(const std::string nickname) const throw();
		void				banUser(const std::string nickname);
		bool				isUserBanned(const std::string nickname) const throw();
		const std::string	getTopic() const throw();
		void				setTopic(const std::string &topic_str);
		int					getModeFlags();
		void				setModeFlag(int flag);
		void				clearModeFlag(int flag);
		void				Channel_commands(IRCUser &user, const IRCMessage &msg);
		
	private:
		// Class contains const member, should not be assignable. 
		IRCChannel &operator=(const IRCChannel &other);

		int											channelmodes;
		const std::string							name;
		std::string*								topic;
		std::string									key;
		UsersList 									users;
		UsersList 									muted_users;
		UsersList 									banned_users;
		static IRCChannelModesMap					flags_enum;
		std::string									mode_str;

	//friend class IRCServer;
};

#endif
