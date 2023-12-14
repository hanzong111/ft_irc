#ifndef FT_IRC_IRC_NUMERIC_REPLIES_HPP
# define FT_IRC_IRC_NUMERIC_REPLIES_HPP

# include <string>

//------------------SECTION TO DEFINE GENERIC VARIADIC MACROS-----------------//
// The following section is taken from the following page:
// stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// Which is in turn taken from the following page:
// groups.google.com/g/comp.std.c/c/d-6Mj5Lko_s?pli=1
// get number of arguments with __NARG__
# define __NARG__(...)  __NARG_I_(__VA_ARGS__,__RSEQ_N())
# define __NARG_I_(...) __ARG_N(__VA_ARGS__)
# define __ARG_N( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,N,...) N
# define __RSEQ_N() \
     63,62,61,60,                   \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0

// general definition for any function name
# define _VFUNC_(name, n) name##n
# define _VFUNC(name, n) _VFUNC_(name, n)
# define VFUNC(func, ...) _VFUNC(func, __NARG__(__VA_ARGS__)) (__VA_ARGS__)
//------------------------------END OF SECTION--------------------------------//

// general format for numeric replies
# define IRC_RPL(numeric_code, server, target) \
    std::string(":") + server + " " + numeric_code + " " + target

//---------------------------------RPL_WELCOME--------------------------------//

/*  handlePASS  */
# define RPL_WELCOME(server, nickname, username, hostname) IRC_RPL("001", server, nickname) + " :Welcome to the IRC network, " + nickname + "!" + username + "@" + hostname + "\r\n"
# define RPL_YOURHOST(server, target, version) IRC_RPL("002", server, target) + " :Your host is " + server + ", running" + " version " + version + "\r\n"
# define RPL_CREATED(server, target, date) IRC_RPL("003", server, target) + " :This server was created " + date + "\r\n"
# define RPL_MYINFO(server, target, version, usermodes, channelmodes) IRC_RPL("004", server, target) + " " + server + " " + version + " " + usermodes + " " + channelmodes + "\r\n"
# define RPL_NOTOPIC(server, target, channel_name) IRC_RPL("331", server, target) + " " + channel_name + " :No topic is set\r\n"
# define RPL_TOPIC(server, target, channel_name, topic_str) IRC_RPL("332", server, target) + " " + channel_name + " :" + topic_str + "\r\n"
/*  handleNICK  */
# define ERR_NONICKNAMEGIVEN(server, target) IRC_RPL("431", server, target) + " :No nickname given\r\n"
# define ERR_ERRONEUSNICKNAME(server, target, requested_nickname) IRC_RPL("432", server, target) + " " + requested_nickname + " :Erroneous nickname\r\n"
# define ERR_NICKNAMEINUSE(server, target, requested_nickname) IRC_RPL("433", server, target) + " " + requested_nickname + " :Nickname is already in use\r\n"
# define ERR_UNAVAILRESOURCE(server, target, param) IRC_RPL("437", server, target) + " " param + " :Nick/channel is temporarily unavailable\r\n"
# define ERR_RESTRICTED(server, target) IRC_RPL("484", server, target) + " :Your connection is restricted!\r\n"
# define ERR_WRONGPASS(server, target, command) IRC_RPL("999", server, target) + " " + command + " :Wrong Password\r\n"
/*  handleOPER  */
# define ERR_NEEDMOREPARAMS(server, target, command) IRC_RPL("461", server, target) + " " + command + " :Not enough parameters\r\n"
# define ERR_ALREADYREGISTRED(server, target) IRC_RPL("462", server, target) + " :Unauthorized command " + "(already registered)\r\n"
# define RPL_YOUREOPER(server, target) IRC_RPL("381", server, target) + " :You are now an IRC operator\r\n"
# define ERR_PASSWDMISMATCH(server, target) IRC_RPL("464", server, target) + " :Password incorrect\r\n"
/*  handleMODE  */
# define ERR_USERSDONTMATCH(server, target) IRC_RPL("502", server, target) + " :Cannot change mode for other users\r\n"
# define RPL_UMODEIS(server, target, mode) IRC_RPL("221", server, target) + " " + mode + "\r\n"
# define ERR_UMODEUNKNOWNFLAG(server, target) IRC_RPL("501", server, target) + " :Unknown MODE flag\r\n"
# define RPL_CHANNELMODEIS(server, target, channel, mode) IRC_RPL("324", server, target) + " " + channel + " " + mode + "\r\n"
# define ERR_USERNOTINCHANNEL(server, target, channelname) IRC_RPL("441", server, target) + " " + target + " " + channelname + " :They aren't on that channel\r\n"
# define ERR_KEYSET(server, target, channelname) IRC_RPL("467", server, target) + " " + channelname + " :Channel key already set\r\n"
# define ERR_UNKNOWNMODE(server, target, channelname, param) IRC_RPL("472", server, target) + " " + param + " :is unknown mode char to me for " + channelname + "\r\n"
/*  handleJOIN  */
# define ERR_NOSUCHCHANNEL(server, target, channelname) IRC_RPL("403", server, target) + " " + channelname + " :No such channel\r\n"
# define ERR_BADCHANNELKEY(server, target, channelname) IRC_RPL("475", server, target) + " " + channelname + " :Cannot join channel (+k)\r\n"
# define ERR_BANNEDFROMCHAN(server, target, channelname)IRC_RPL("474", server, target) + " " + channelname + " :Cannot join channel (+b)\r\n"
# define RPL_NAMREPLY(server, target, symbol, channel, list_of_nicks) (IRC_RPL("353", server, target) + " " + symbol + " " + channel + " :" + list_of_nicks + "\r\n")
/*   handlePRIVMSG  */
# define ERR_CANNOTSENDTOCHAN(server, target, channel) (IRC_RPL("404", server, target) + " " + channel + " :Cannot send to channel\r\n")
# define ERR_NORECIPIENT(server, target, command) (IRC_RPL("411", server, target) + " :No recipient given (" + command + ")\r\n")
# define ERR_NOTEXTTOSEND(server, target) (IRC_RPL("412", server, target) + " :No text to send\r\n")
# define ERR_NOSUCHNICK(server, target, param) (IRC_RPL("401", server, target) + " " + param + " :No such nick/channel\r\n")

/*   handlePART     */
# define ERR_NOTONCHANNEL(server, target, channel) (IRC_RPL("442", server, target) + " " + channel + " :You're not on that channel\r\n")
# define ERR_CHANOPRIVSNEEDED(server, target, channel) (IRC_RPL("482", server, target) + " " + channel + " :You're not channel operator\r\n")

#endif
