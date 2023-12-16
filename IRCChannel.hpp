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
 	C_TOPIC = 0x200,
    C_BANNED = 0x400,
    C_MUTED = 0x800,
    C_OPER = 0x1000,
    C_CREATOR = 0x2000,
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
		std::string			getUsersAsStr(char separator) const throw();
		const std::string	&getModestr();
		const std::string	&getKey();
		IRCChannelModesMap 	&getFlag_map();
		void				addUser(const std::string &nickname);
		void				removeUser(const std::string &nickname);
		void				muteUser(const std::string &nickname);
		void				banUser(const std::string &nickname);
		const std::string	getTopic() const throw();
		void				setTopic(const std::string &topic_str);
		int					getModeFlags();
		void				setModeFlag(int flag);
		void				clearModeFlag(int flag);
		void				setCreator(const std::string &user);
		void				addOper(const std::string &nickname);
		void				removeOper(const std::string &nickname);
		void				setLimit(int value);
		int					getLimit() const throw();
		void				clearLimit();
		void				setKey(const std::string &key);
		void				removeKey();
		size_t				getNumUsers() const throw();
		int					getintNumUsers() const throw();
		
		bool				isTopicset() const throw();
		bool				isUserBanned(const std::string &nickname) const throw();
		bool				isUserInChannel(const std::string &nickname) const throw();
		bool				isUserMuted(const std::string &nickname) const throw();
		bool				isUserOper(const std::string &nickname) const throw();
		bool				isKeyset() const throw();
		bool				isLimitset() const throw();
		bool				isUserCreator(const std::string &nickname) const throw();
		void				print_opers();
		void				print_users();
		const	std::string	&getCreator() const throw();
		void				removeBannedUser(const std::string &nickname);
		void				removeMutedUser(const std::string &nickname);

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
		UsersList									channel_opers;
		static IRCChannelModesMap					flags_enum;
		std::string									mode_str;
		std::string									creator;
		int											limit;

	//friend class IRCServer;
};

#endif
