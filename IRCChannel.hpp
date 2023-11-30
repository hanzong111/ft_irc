#ifndef FT_IRC_IRCCHANNEL_HPP
# define FT_IRC_IRCCHANNEL_HPP

# include <string>
# include <set>

class IRCChannel
{
	typedef std::set<std::string> UsersList;

	public:
		IRCChannel(const std::string &channel_name);
		IRCChannel(const IRCChannel &other);
		virtual ~IRCChannel();

		const std::string	&getName() const throw();
		const UsersList		&getUsers() const throw();
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
		
	private:
		// Class contains const member, should not be assignable. 
		IRCChannel &operator=(const IRCChannel &other);

		int					modes;
		const std::string	name;
		std::string			*topic;
		UsersList 			users;
		UsersList 			muted_users;
		UsersList 			banned_users;

	//friend class IRCServer;
};

#endif
