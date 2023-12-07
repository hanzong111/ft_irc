#include "IRCUser.hpp"
#include "TCPConn.hpp"
#include "IRCServer.hpp"
#include "charset.h"
#include <string.h>
#include <sstream>

uint64_t IRCUser::count = 0;

IRCUser::IRCUser(const IRCServer &server) :
	TCPConn(server),
	is_authenticated(false),
	is_registered(false),
	is_operator(false),
	usermode_str("+")
{
	std::ostringstream os;
	os << IRCUSER_DEFAULT_NICK_PREFIX << count++;
	nickname = os.str();
}

IRCUser::IRCUser(const TCPServer<IRCUser> &server) :
	TCPConn(server),
	is_authenticated(false),
	is_registered(false),
	is_operator(false),
	usermode_str("+")
{
	std::ostringstream os;
	os << IRCUSER_DEFAULT_NICK_PREFIX << count++;
	nickname = os.str();
}

IRCUser::IRCUser(const IRCUser &other) :
	TCPHost(other),
	TCPConn(other),
	is_authenticated(other.is_authenticated),
	is_registered(other.is_registered),
	is_operator(other.is_operator),
	usermode_str(other.usermode_str),
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
	is_operator = other.is_operator;
	nickname = other.nickname;
	username = other.username;
	real_name = other.real_name;
	usermode_str = other.usermode_str;
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

std::string	&IRCUser::getModeFlags()
{
	return (usermode_str);
}

void	IRCUser::setModeFlag(std::string const &newflag)
{
	usermode_str += newflag;
}

void	IRCUser::clearModeFlag(std::string &rmflag)
{
	std::string::size_type found = usermode_str.find(rmflag);
	if (found != std::string::npos)
		usermode_str.erase(found, 1);
}

bool	IRCUser::isAuthenticated() const throw()
{
	return (is_authenticated);
}

bool	IRCUser::isRegistered() const throw()
{
	return (is_registered);
}

bool	IRCUser::isOperator() const throw()
{
	return (is_operator);
}


void	IRCUser::makeAuthenticated() throw()
{
	is_authenticated = true;
}

void	IRCUser::makeRegistered() throw()
{
	is_registered = true;
}

void	IRCUser::makeOperator() throw()
{
	is_operator = true;
}

