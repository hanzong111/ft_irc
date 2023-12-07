#ifndef FT_IRC_IRCUSER_HPP
# define FT_IRC_IRCUSER_HPP

# include "TCPConn.hpp"
# include <string>

# define IRCUSER_DEFAULT_NICK_PREFIX "@DEF"

template <typename ClientType>
class TCPServer;
class IRCServer;

typedef enum IRCUserModes
{
	AWAY = 0x1,
	RESTRICTED = 0x2,
	WALLOPS = 0x4,
	INVISIBLE = 0x8,
	OPER = 0x10,
	LOCAL_OPER = 0x20,
	SERVER_NOTICES = 0x40
} e_IRCUserModes;

class IRCUser : public TCPConn
{
	public:
		// Default constructor does not exist, because IRCUser shouldn't exist
		// without association to any server.
		IRCUser(const IRCServer &server);
		IRCUser(const TCPServer<IRCUser> &server);
		IRCUser(const IRCUser &other);
		IRCUser &operator=(const IRCUser &other);
		virtual ~IRCUser();

		static bool			isValidNickname(const std::string &str) throw();

		const std::string	&getNickname() const throw();
		const std::string	&getUsername() const throw();
		const std::string	&getRealName() const throw();
		std::string			changeNickname(const std::string &new_nickname);
		std::string			changeUsername(const std::string &new_username);
		std::string			changeRealName(const std::string &new_real_name);
		bool				isAuthenticated() const throw();
		bool				isRegistered() const throw();
		bool				isOperator() const throw();
		void				makeAuthenticated() throw();
		void				makeRegistered() throw();
		void				makeOperator() throw ();
		std::string			&getModeFlags();
		void				setModeFlag(std::string const &newflag);
		void				clearModeFlag(std::string &rmflag);

	protected:
		static uint64_t	count;
		bool			is_authenticated;
		bool			is_registered;
		bool			is_operator;
		std::string		usermode_str;
		std::string		nickname;
		std::string		username;
		std::string		real_name;
};

#endif
