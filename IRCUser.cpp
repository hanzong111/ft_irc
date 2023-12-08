#include "IRCUser.hpp"
#include "TCPConn.hpp"
#include "IRCServer.hpp"
#include "charset.h"
#include <string.h>
#include <sstream>

uint64_t IRCUser::count = 0;

IRCUserModesMap IRCUser::flags_enum;

void	populateModeMap(IRCUserModesMap &map)
{
	map['a'] = AWAY;
	map['r'] = RESTRICTED;
	map['w'] = WALLOPS;
	map['i'] = INVISIBLE;
	map['o'] = OPER;
	map['O'] = LOCAL_OPER;
	map['s'] = SERVER_NOTICES;
}

IRCUser::IRCUser(const IRCServer &server) :
	TCPConn(server),
	is_authenticated(false),
	is_registered(false),
	usermodes(0),
	mode_str("+")
{
	std::ostringstream os;
	os << IRCUSER_DEFAULT_NICK_PREFIX << count++;
	nickname = os.str();
	if (flags_enum.empty())
		populateModeMap(flags_enum);
}

IRCUser::IRCUser(const TCPServer<IRCUser> &server) :
	TCPConn(server),
	is_authenticated(false),
	is_registered(false),
	usermodes(0),
	mode_str("+")
{
	std::ostringstream os;
	os << IRCUSER_DEFAULT_NICK_PREFIX << count++;
	nickname = os.str();
	if (flags_enum.empty())
		populateModeMap(flags_enum);
}

IRCUser::IRCUser(const IRCUser &other) :
	TCPHost(other),
	TCPConn(other),
	is_authenticated(other.is_authenticated),
	is_registered(other.is_registered),
	usermodes(other.usermodes),
	mode_str(other.mode_str),
	nickname(other.nickname),
	username(other.username),
	real_name(other.real_name)
{}

IRCUser &IRCUser::operator=(const IRCUser &other)
{
	if (this == &other)
		return (*this);
	TCPConn::operator=(other);
	is_authenticated = other.is_authenticated;
	is_registered = other.is_registered;
	nickname = other.nickname;
	username = other.username;
	real_name = other.real_name;
	usermodes = other.usermodes;
	mode_str = other.mode_str;
	flags_enum = other.flags_enum;
	return (*this);
}

IRCUser::~IRCUser()
{}

bool	IRCUser::isValidNickname(const std::string &str) throw()
{
	// If str is empty or if str has more than 9 characters
	if (str.size() < 1 || str.size() > 9)
		return (false);
	// If first character isn't a letter or a special character
	if (std::string(CHARSET_LETTERS CHARSET_SPECIAL).find(str[0]) == str.npos)
		return (false);
	// If any of the character following the first is none of letters, digits, special chracters,
	// and '-'
	if (str.find_first_not_of(std::string(CHARSET_LETTERS CHARSET_DIGITS CHARSET_SPECIAL "-"), 1) 
			!= str.npos)
		return (false);
	return (true);
}

const std::string	&IRCUser::getNickname() const throw()
{
	return (nickname);
}

const std::string	&IRCUser::getUsername() const throw()
{
	return (username);
}

const std::string	&IRCUser::getRealName() const throw()
{
	return (real_name);
}

IRCUserModesMap &IRCUser::getFlag_map()
{
	return (flags_enum);
}

std::string	IRCUser::changeNickname(const std::string &new_nickname)
{
	std::string	old_nickname = nickname;

	nickname = new_nickname;
	return (old_nickname);
}

std::string IRCUser::changeUsername(const std::string &new_username)
{
	std::string	old_username = username;

	username = new_username;
	return (old_username);
}

std::string	IRCUser::changeRealName(const std::string &new_real_name)
{
	std::string	old_real_name = real_name;

	real_name = new_real_name;
	return (old_real_name);
}

int	IRCUser::getModeFlags()
{
	return (usermodes);
}

void	IRCUser::setModeFlag(int flag)
{
	usermodes |= flag;
}

void	IRCUser::clearModeFlag(int flag)
{
	usermodes &= ~flag;
}

bool	IRCUser::isAuthenticated() const throw()
{
	return (is_authenticated);
}

bool	IRCUser::isRegistered() const throw()
{
	return (is_registered);
}

void	IRCUser::makeAuthenticated() throw()
{
	is_authenticated = true;
}

void	IRCUser::makeRegistered() throw()
{
	is_registered = true;
}

const std::string	&IRCUser::getModestr()
{
	mode_str.clear();
	mode_str = "+";
	for (IRCUserModesMap::iterator it = flags_enum.begin(); it != flags_enum.end(); ++it)
	{
        if(usermodes & it->second)
			mode_str += it->first;
    }
	return (mode_str);
}
