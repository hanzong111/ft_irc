#ifndef FT_IRC_TCPCLIENT_HPP
# define FT_IRC_TCPCLIENT_HPP

# include "TCPHost.hpp"
# include <poll.h>
# include <string>
# include <queue>
# include <netinet/in.h>

# define TCPSERVER_INIT_BUF_SIZE 1024

class TCPServer;

class TCPClient : virtual public TCPHost
{
	public:
		TCPClient(const TCPServer &server);
		TCPClient(const TCPClient &other);
		TCPClient &operator=(const TCPClient &other);
		virtual ~TCPClient();

		ssize_t		sendBytes(const void *buf, size_t n, int flags) const;		
		ssize_t		recvBytes(void *buf, size_t n, int flags) const;
		void		queueSend(const void* buf, size_t n);
		ssize_t		processSendQueue(int flags);
		ssize_t		requestRecv(const char* delimiter, size_t delimiter_size, int flags);
		bool 		retrieveRecvBuf(char *buffer, size_t *size);
		short		getPollREvents() const throw();
		void		setPollREvents(short revents) throw();
		short		getPollEvents() const throw();
		void		setPollEvents(short events) throw();
		void		setPollFdPtr(pollfd *ptr) throw();

	protected:
		char	*recv_buf;
		size_t	recv_buf_size;
		size_t	recv_data_size;
		size_t	recv_retrieve_size;
		bool	partial_receive;
		pollfd	*pollfd_struct;
		std::queue< std::pair<char *, size_t> >	send_queue;

	private:
		TCPClient();
	
	//friend class TCPServer;
};

#endif