#include "IRCChannel.hpp"
#include <string>
#include <utility>

IRCChannelModesMap IRCChannel::flags_enum;

void	populateModeMap(IRCChannelModesMap &map)
{
	map['c'] = C_KEY;
	map['l'] = C_LIMIT;
	map['o'] = C_OPER;
	map['O'] = C_CREATOR;
}

IRCChannel::IRCChannel(const std::string &channel_name) :
	channelmodes(0),
	name(channel_name),
	topic(NULL),
	key(NULL),
	mode_str("+")
{
	banned_users = std::set<std::string>();
	muted_users = std::set<std::string>();
	users = std::set<std::string>();
	if (flags_enum.empty())
		populateModeMap(flags_enum);
}

IRCChannel::IRCChannel(const std::string &channel_name, const std::string &channel_key) :
	channelmodes(0),
	name(channel_name),
	topic(NULL),
	key(channel_key),
	mode_str("+")
{
	banned_users = std::set<std::string>();
	muted_users = std::set<std::string>();
	users = std::set<std::string>();
	if (flags_enum.empty())
		populateModeMap(flags_enum);
	if(!channel_key.empty())
		setModeFlag(C_KEY);
}

IRCChannel::IRCChannel(const IRCChannel &other) :
	channelmodes(other.channelmodes),
	name(other.name),
	key(other.key),
	users(other.users),
	muted_users(other.muted_users),
	banned_users(other.banned_users),
	mode_str("+")
{
	if (other.topic)
		topic = new std::string(*(other.topic));
	else
		topic = NULL;
}

IRCChannel::~IRCChannel()
{
	delete (topic);
}

const std::string	&IRCChannel::getName() const throw()
{
	return (name);
}

const IRCChannel::UsersList	&IRCChannel::getUsers() const throw()
{
	return (users);
}

void	IRCChannel::addUser(const std::string nickname)
{
	//std::pair<UsersList::iterator, bool>	result;

	//result = users.insert(nickname);
	if (banned_users.find(nickname) == banned_users.end())
		users.insert(nickname);
}

void	IRCChannel::removeUser(const std::string nickname)
{
	users.erase(nickname);
}

bool	IRCChannel::isUserInChannel(const std::string nickname) const throw()
{
	return (users.find(nickname) != users.end());
}

void	IRCChannel::muteUser(const std::string nickname)
{
	muted_users.insert(nickname);
}

bool	IRCChannel::isUserMuted(const std::string nickname) const throw()
{
	if(!muted_users.empty())
		return (muted_users.find(nickname) != muted_users.end());
	else
		return(false);
}

void	IRCChannel::banUser(const std::string nickname)
{
	banned_users.insert(nickname);
}

bool	IRCChannel::isUserBanned(const std::string nickname) const throw()
{
	if (banned_users.size() == 0)
		return (false);
	if(banned_users.size() > 0)
		return (banned_users.find(nickname) != banned_users.end());
	else
		return(false);
}

const std::string	IRCChannel::getTopic() const throw()
{
	if (topic == NULL)
		return ("");
	else
		return (*topic);
}

void	IRCChannel::setTopic(const std::string &topic_str)
{
	delete (topic);
	topic = new std::string(topic_str);
}

int	IRCChannel::getModeFlags()
{
	return (channelmodes);
}

void	IRCChannel::setModeFlag(int flag)
{
	channelmodes |= flag;
}

void	IRCChannel::clearModeFlag(int flag)
{
	channelmodes &= ~flag;
}

IRCChannelModesMap &IRCChannel::getFlag_map()
{
	return (flags_enum);
}

const std::string	&IRCChannel::getModestr()
{
	mode_str.clear();
	mode_str = "+";
	for (IRCChannelModesMap::iterator it = flags_enum.begin(); it != flags_enum.end(); ++it)
	{
        if(channelmodes & it->second)
			mode_str += it->first;
    }
	return (mode_str);
}

const std::string	&IRCChannel::getKey()
{
	return(key);
}
