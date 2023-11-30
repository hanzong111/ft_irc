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
/* Usage:
 * RPL_YOURHOST(servername, nickname, username, hostname)
 */
# define RPL_WELCOME(server, nickname, username, hostname) \
    IRC_RPL("001", server, nickname) + " :Welcome to the IRC network, " \
		+ nickname + "!" + username + "@" + hostname + "\r\n"
//------------------------------END OF SECTION--------------------------------//

//--------------------------------RPL_YOURHOST--------------------------------//
/* Usage:
 * RPL_YOURHOST(servername, nickname, version_str)
 */
# define RPL_YOURHOST(server, target, version) \
    IRC_RPL("002", server, target) + " :Your host is " + server + ", running" \
		+ " version " + version + "\r\n"
//------------------------------END OF SECTION--------------------------------//

//---------------------------------RPL_CREATED--------------------------------//
/* Usage:
 * RPL_CREATED(servername, nickname, date_str)
 */
# define RPL_CREATED(server, target, date) \
    IRC_RPL("003", server, target) + " :This server was created " + date \
        + "\r\n"
//------------------------------END OF SECTION--------------------------------//

//---------------------------------RPL_MYINFO---------------------------------//
/* Usage:
 * RPL_MYINFO(servername, nickname, version_str, usermodes, channelmodes)
 */
# define RPL_MYINFO(server, target, usermodes, channelmodes) \
    IRC_RPL("004", server, target) + " " + server + " " + usermodes + " " \
		+ channelmodes + "\r\n"
//------------------------------END OF SECTION--------------------------------//

//---------------------------------RPL_NOTOPIC--------------------------------//
/* Usage:
 * RPL_NOTOPIC(servername, nickname, channel_name)
 */
# define RPL_NOTOPIC(server, target, channel_name) \
    IRC_RPL("331", server, target) + " " + channel_name \
        + " :No topic is set\r\n"
//------------------------------END OF SECTION--------------------------------//

//----------------------------------RPL_TOPIC---------------------------------//
/* Usage:
 * RPL_TOPIC(servername, nickname, channel_name, topic_str)
 */
# define RPL_TOPIC(server, target, channel_name, topic_str) \
    IRC_RPL("331", server, target) + " " + channel_name + " :" + topic_str \
         + "\r\n"
//------------------------------END OF SECTION--------------------------------//

//-----------------------------ERR_NONICKNAMEGIVEN----------------------------//
/* Usage:
 * ERR_NONICKNAMEGIVEN(servername, nickname)
 */
# define ERR_NONICKNAMEGIVEN(server, target) \
    IRC_RPL("431", server, target) + " :No nickname given\r\n"
//------------------------------END OF SECTION--------------------------------//

//----------------------------ERR_ERRONEUSNICKNAME----------------------------//
/* Usage:
 * ERR_ERRONEUSNICKNAME(servername, nickname, requested_nickname)
 */
# define ERR_ERRONEUSNICKNAME(server, target, requested_nickname) \
    IRC_RPL("432", server, target) + requested_nickname \
        + " :Erroneous nickname\r\n"
//------------------------------END OF SECTION--------------------------------//

//------------------------------ERR_NICKNAMEINUSE-----------------------------//
/* Usage:
 * ERR_NICKNAMEINUSE(servername, nickname, requested_nickname)
 */
# define ERR_NICKNAMEINUSE(server, target, requested_nickname) \
    IRC_RPL("433", server, target) + requested_nickname \
        + " :Nickname is already in use\r\n"
//------------------------------END OF SECTION--------------------------------//

//-----------------------------ERR_UNAVAILRESOURCE----------------------------//
/* Usage:
 * ERR_UNAVAILRESOURCE(servername, nickname, param)
 */
# define ERR_UNAVAILRESOURCE(server, target, param) \
    IRC_RPL("437", server, target) + param \
        + " :Nick/channel is temporarily unavailable\r\n"
//------------------------------END OF SECTION--------------------------------//

//-------------------------------ERR_RESTRICTED-------------------------------//
/* Usage:
 * ERR_RESTRICTED(servername, nickname)
 */
# define ERR_RESTRICTED(server, target) \
    IRC_RPL("484", server, target) + " :Your connection is restricted!\r\n"
//------------------------------END OF SECTION--------------------------------//

//-----------------------------ERR_NEEDMOREPARAMS-----------------------------//
/* Usage:
 * ERR_NEEDMOREPARAMS(servername, nickname, command)
 */
# define ERR_NEEDMOREPARAMS(server, target, command) \
    IRC_RPL("461", server, target) + " " + command \
        + " :Not enough parameters\r\n"
//------------------------------END OF SECTION--------------------------------//

//----------------------------ERR_ALREADYREGISTRED----------------------------//
/* Usage:
 * ERR_ALREADYREGISTRED(servername, nickname)
 */
# define ERR_ALREADYREGISTRED(server, target) \
    IRC_RPL("462", server, target) + " :Unauthorized command " \
		+ "(already registered)\r\n"
//------------------------------END OF SECTION--------------------------------//

#endif
