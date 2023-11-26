#include "catch_amalgamated.hpp"
#include "../IRCMessage.hpp"
#include <iostream>

TEST_CASE("IRCMessage default constructor creates an empty message") {
    IRCMessage msg;
    REQUIRE(msg.prefix == "");
    REQUIRE(msg.command == "");
    REQUIRE(msg.params.empty());
}

TEST_CASE("IRCMessage constructor with full message string") {
    std::string fullMessage = ":nick!user@host PRIVMSG #channel :Hello, world!\r\n";
    IRCMessage msg(fullMessage);
    REQUIRE(msg.prefix == "nick!user@host");
    REQUIRE(msg.command == "PRIVMSG");
    REQUIRE(msg.params.size() == 2);
    REQUIRE(msg.params[0] == "#channel");
    REQUIRE(msg.params[1] == "Hello, world!");
}

TEST_CASE("IRCMessage constructor with message without prefix but with 2 params") {
    std::string messageWithoutPrefix = "PRIVMSG #channel :Hello, world!\r\n";
    IRCMessage msg(messageWithoutPrefix);
    REQUIRE(msg.prefix == "");
    REQUIRE(msg.command == "PRIVMSG");
    REQUIRE(msg.params.size() == 2);
    REQUIRE(msg.params[0] == "#channel");
    REQUIRE(msg.params[1] == "Hello, world!");
}

TEST_CASE("IRCMessage constructor with message without prefix but with 1 param") {
    std::string messageWithoutPrefix = "QUIT :Leaving the chat\r\n";
    IRCMessage msg(messageWithoutPrefix);
    REQUIRE(msg.prefix == "");
    REQUIRE(msg.command == "QUIT");
    REQUIRE(msg.params.size() == 1);
    REQUIRE(msg.params[0] == "Leaving the chat");
}

TEST_CASE("IRCMessage constructor with message without prefix and parameter") {
    std::string messageWithoutPrefix = "QUIT\r\n";
    IRCMessage msg(messageWithoutPrefix);
    REQUIRE(msg.prefix == "");
    REQUIRE(msg.command == "QUIT");
    REQUIRE(msg.params.size() == 0);
}

TEST_CASE("IRCMessage constructor with message without params") {
    std::string messageWithoutParams = ":nick!user@host PRIVMSG\r\n";
    IRCMessage msg(messageWithoutParams);
    REQUIRE(msg.prefix == "nick!user@host");
    REQUIRE(msg.command == "PRIVMSG");
    REQUIRE(msg.params.empty());
}

TEST_CASE("IRCMessage constructor with message with 15 params (last param contain space)") {
    std::string messageWithoutParams = ":nick!user@host HYPOTHETICAL "
		"1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\r\n";
    IRCMessage msg(messageWithoutParams);
    REQUIRE(msg.prefix == "nick!user@host");
    REQUIRE(msg.command == "HYPOTHETICAL");
	REQUIRE(msg.params.size() == 15);
	for (size_t i = 0; i <= 13; i++)
		REQUIRE(msg.params[i] == std::to_string(i + 1));
	REQUIRE(msg.params[15 - 1] == "15 16");
}

TEST_CASE("IRCMessage copy constructor") {
    IRCMessage original(":nick!user@host PRIVMSG #channel :Hello, world!\r\n");
    IRCMessage copy(original);
    REQUIRE(copy.prefix == original.prefix);
    REQUIRE(copy.command == original.command);
    REQUIRE(copy.params == original.params);
}

TEST_CASE("IRCMessage copy assignment operator") {
    IRCMessage original(":nick!user@host PRIVMSG #channel :Hello, world!\r\n");
    IRCMessage copy;
    copy = original;
    REQUIRE(copy.prefix == original.prefix);
    REQUIRE(copy.command == original.command);
    REQUIRE(copy.params == original.params);
}
