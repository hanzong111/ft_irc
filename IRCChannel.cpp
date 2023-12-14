#include "IRCChannel.hpp"
#include <string>
#include <utility>

IRCChannelModesMap IRCChannel::flags_enum;

void	populateModeMap(IRCChannelModesMap &map)
{
	map['k'] = C_KEY;
	map['l'] = C_LIMIT;
	map['t'] = C_TOPIC;
	map['b'] = C_BANNED;
	map['m'] = C_MUTED;
	map['o'] = C_OPER;
	map['O'] = C_CREATOR;
}

IRCChannel::IRCChannel(const std::string &channel_name) :
	channelmodes(0),
	name(channel_name),
	topic(NULL),
	key(""),
	mode_str("+"),
	creator(""),
	limit(-1)
{
	banned_users = std::set<std::string>();
	muted_users = std::set<std::string>();
	users = std::set<std::string>();
	channel_opers = std::set<std::string>();
	if (flags_enum.empty())
		populateModeMap(flags_enum);
}

IRCChannel::IRCChannel(const std::string &channel_name, const std::string &channel_key) :
	channelmodes(0),
	name(channel_name),
	topic(NULL),
	key(channel_key),
	mode_str("+"),
	creator(""),
	limit(-1)
{
	banned_users = std::set<std::string>();
	muted_users = std::set<std::string>();
	users = std::set<std::string>();
	channel_opers = std::set<std::string>();
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
	channel_opers(other.channel_opers),
	mode_str("+"),
	creator(other.creator),
	limit(other.limit)
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
	if(topic_str == "\"\"")
	{
		topic = NULL;
		clearModeFlag(C_TOPIC);
	}
	else
	{
		topic = new std::string(topic_str);
		setModeFlag(C_TOPIC);
	}
}

bool	IRCChannel::isTopicset() const throw()
{
	if(channelmodes & C_TOPIC)
		return(true);
	else
		return(false);
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

void	IRCChannel::setCreator(const std::string &user)
{
	creator = user;
}

void	IRCChannel::addOper(const std::string nickname)
{
	if (banned_users.find(nickname) == banned_users.end())
		channel_opers.insert(nickname);
}

void	IRCChannel::removeOper(const std::string nickname)
{
	channel_opers.erase(nickname);
}

bool	IRCChannel::isUserOper(const std::string nickname) const throw()
{
	return (channel_opers.find(nickname) != channel_opers.end());
}

bool	IRCChannel::isKeyset() const throw()
{
	if(channelmodes & C_KEY)
		return(true);
	else
		return(false);
}

bool	IRCChannel::isUserCreator(const std::string nickname) const throw()
{
	if(nickname == creator)
		return(true);
	else
		return(false);
}

bool	IRCChannel::isLimitset() const throw()
{
	if(channelmodes & C_LIMIT)
		return(true);
	else
		return(false);
}

void	IRCChannel::setLimit(int value)
{
	limit = value;
	setModeFlag(C_LIMIT);
}

int		IRCChannel::getLimit() const throw()
{
	return(limit);
}

void	IRCChannel::clearLimit()
{
	limit = -1;
}

void	IRCChannel::setKey(const std::string &key_toset)
{
	key = key_toset;
	setModeFlag(C_KEY);
}

void	IRCChannel::removeKey()
{
	key.clear();
	clearModeFlag(C_KEY);
}