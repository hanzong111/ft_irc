#include "TCPConn.hpp"
#include "TCPHost.hpp"
#include "TCPServer.hpp"
#include "TCPServer_impl.hpp"
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <string>
#include <cerrno>
#include <algorithm>
#include <iostream>
#include <netinet/in.h>

TCPConn::TCPConn() :
	TCPHost()
{
	recv_buf = new char[TCPSERVER_INIT_BUF_SIZE];
	recv_buf_size = TCPSERVER_INIT_BUF_SIZE; 
	recv_data_size = 0;
	recv_retrieve_size = 0;
	partial_receive = false;
	pollfd_struct = NULL;
}

/**
 * @note Copies will share the same pollfd struct, and sending may not work 
 * correctly since the copies will all have separate send_queue and the POLLOUT 
 * bit in the `event` field of the pollfd struct will be cleared after a copy 
 * has completed its send. 
*/
TCPConn::TCPConn(const TCPConn &other) :
	TCPHost()
{
	recv_buf = NULL;
	recv_buf_size = 0;
	pollfd_struct = other.pollfd_struct;
	*this = other;
}

TCPConn &TCPConn::operator=(const TCPConn &other)
{
	if (this == &other)
		return (*this);
	if (fd != -1)
	{
		std::cout << "Closing socket: " << fd << std::endl;
		if (close(fd) == -1)
			std::cerr << "Warning: Failed to close socket" << std::endl;
	}
	fd = dup(other.fd);
	if (fd == -1)
		throw std::runtime_error(std::string("dup: ") + strerror(errno));
	addr = other.addr;
	addr_len = other.addr_len;
	if (recv_buf_size < other.recv_buf_size)
	{
		delete[] recv_buf;
		recv_buf = new char[other.recv_buf_size];
		recv_buf_size = other.recv_buf_size;
	}
	memcpy(recv_buf, other.recv_buf, other.recv_buf_size);
	recv_data_size = other.recv_data_size;
	recv_retrieve_size = other.recv_retrieve_size;
	partial_receive = other.partial_receive;
	if (pollfd_struct != NULL && other.pollfd_struct != pollfd_struct)
	{
		memcpy(pollfd_struct, other.pollfd_struct, sizeof(*pollfd_struct));
		pollfd_struct->fd = fd;
	}

	// Delete data in send_queue
	while (!send_queue.empty())
	{
		delete[] send_queue.front().first;
		send_queue.pop();
	}
	// Copy send_queue
	std::pair<char *, size_t> tmp;
	std::queue< std::pair<char *, size_t> > queue_cpy = other.send_queue;
	while (!queue_cpy.empty())
	{
		tmp.first = new char[queue_cpy.front().second];
		tmp.second = queue_cpy.front().second;
		memcpy(tmp.first, queue_cpy.front().first, tmp.second);
		send_queue.push(tmp);
	}
	return (*this);
}

TCPConn::~TCPConn()
{
	delete[] recv_buf;
	// Delete data in send_queue
	while (!send_queue.empty())
	{
		delete[] send_queue.front().first;
		send_queue.pop();
	}
	if (fd != -1)
	{
		std::cout << "Closing socket: " << fd << std::endl;
		if (close(fd) == -1)
			std::cerr << "Warning: Failed to close socket" << std::endl;
	}
}

/**
 * @brief A simple wrapper for `send()`.
 */
ssize_t	TCPConn::sendBytes(const void* buf, size_t n, int flags) const
{
	return(send(fd, buf, n, flags));
}

/**
 * @brief A simple wrapper for `recv()`.
 */
ssize_t		TCPConn::recvBytes(void *buf, size_t n, int flags) const
{
	return (recv(fd, buf, n, flags));
}

/**
 * @brief Queues a send operation in the TCP client.
 *
 * This function allocates memory to store the provided data and adds
 * the data to an internal send_queue for later transmission. If the size of the
 * data is zero, the function returns early without modifying the queue.
 *
 * @param buf A pointer to the data to be sent.
 * @param n The size, in bytes, of the data to be sent.
 *
 * @note The memory for the data is dynamically allocated using `new char[]`.
 *	   The caller is responsible for managing the lifetime of `buf`.
 *	   The allocated memory will be freed when the data is sent or when the TCP
 *	   client object is destroyed, ensuring proper cleanup.  
 */
void	TCPConn::queueSend(const void *buf, size_t n)
{
	char	*new_buf;

	if (n == 0)
		return ;
	new_buf = new char[n];
	std::memcpy(new_buf, buf, n);
	send_queue.push(std::pair<char *, size_t>(new_buf, n));
	pollfd_struct->events |= POLLOUT;
}

/**
 * @brief Processes the send queue for the TCP client.
 *
 * This function attempts to send the data in the front of the send queue
 * for the TCP client until the entire buffer is sent or an error occurs.
 *
 * @param flags Additional flags to control the behavior of the send operation.
 * @return On success, it returns the total number of bytes sent. On error, it returns -1.
 *
 * @note If the send operation is successful, the sent data is removed from the internal
 * queue. If the operation is not completed, the caller is expected to call this function
 * again in the future to complete the operation (until a positive value is returned).
 */
ssize_t	TCPConn::processSendQueue(int flags)
{
	size_t	total_bytes_sent = 0;
	ssize_t	n_bytes_sent = 0;
	char	*buf;
	size_t	buf_size;

	if (!(pollfd_struct->revents & POLLOUT))
		return (-1);
	pollfd_struct->revents &= ~POLLOUT;
	buf = send_queue.front().first;
	buf_size = send_queue.front().second;
	while (total_bytes_sent < buf_size)
	{
		n_bytes_sent = send(fd, buf, buf_size, flags);
		if (n_bytes_sent > 0) // Handle successful send
		{
			total_bytes_sent += n_bytes_sent;
			buf_size -= n_bytes_sent;
			if (buf_size != 0) // Partial send
				std::memmove(buf, buf + n_bytes_sent, buf_size);
			else // Send completed
			{
				delete[] buf;
				send_queue.pop();
				if (send_queue.empty())
					pollfd_struct->events &= ~POLLOUT;
				return (total_bytes_sent);
			}
		}
		else
			break ;
	}
	if (total_bytes_sent > 0)
		send_queue.front().second = buf_size;
	return (-1);
}

/**
 * @brief Attempts to receive data into the internal buffer until a specified delimiter sequence is found.
 *
 * This function calls the recv() function to attempt to receive data into the internal buffer.
 * It searches for a specified delimiter sequence and returns size of the data up until the first
 * occurrence of the delimiter (inclusive). If the delimiter is not found (in case of a partial receive),
 * it keeps the data in the buffer for subsequent calls.
 *
 * @param delimiter Pointer to the delimiter sequence.
 * @param delimiter_size Size of the delimiter sequence.
 * @param flags Additional flags to control the behavior of the recv() operation.
 * @return The size of the received data up until the delimiter sequence (inclusive).
 *         Returns -1 if the remote side has closed the connection (graceful shutdown).
 *         Returns 0 if the delimiter is not found (partial receive) or in case of recv() errors.
 *
 * @note The caller should use TCPConn::retrieveRecvBuffer to access the received data.
 */
ssize_t TCPConn::requestRecv(const char* delimiter, size_t delimiter_size, int flags)
{
	if (!(pollfd_struct->revents & POLLIN))
		return (0);
	pollfd_struct->revents &= ~POLLIN;
	// Ensure the existing buffer is large enough
	if (recv_buf_size < recv_data_size + delimiter_size)
	{
		char *new_buf = new char[recv_buf_size << 1];
		std::memcpy(new_buf, recv_buf, recv_data_size);
		delete[] recv_buf;  // Free the existing buffer
		recv_buf = new_buf;
		recv_buf_size = recv_data_size + delimiter_size;
	}

	// Attempt to receive data into the buffer
	ssize_t n_bytes_received = recv(fd, recv_buf + recv_data_size, recv_buf_size - recv_data_size, flags);

	if (n_bytes_received > 0)
	{
		recv_data_size += n_bytes_received;

		// Search for the delimiter in the received data
		const char* found_delimiter = std::search(recv_buf, recv_buf + recv_data_size, delimiter, delimiter + delimiter_size);

		if (found_delimiter != recv_buf + recv_data_size) {

			// Delimiter found, return the size of data up to the delimiter
			recv_retrieve_size = found_delimiter - recv_buf + delimiter_size;
			partial_receive = false;
			return (recv_retrieve_size);
		} else {
			// Delimiter not found (partial receive), set flag for subsequent calls
			partial_receive = true;
			return (0);
		}
	}
	else if (n_bytes_received == 0)
	{
		// Client disconnection
		return (-1);
	}
	else
	{
		// recv() error
		return (0);
	}
}

/**
 * @brief Retrieves the receive buffer and its size, copying data to an external buffer.
 *
 * This function returns a pointer to the receive buffer and its size to the caller,
 * copying the data to the specified external buffer. The internal buffer is updated to
 * remove the data that has been copied. The operation can only be performed if
 * partial receive has not occurred (previous call to TCPConn::requestRecv must have 
 * returned a positive value). The caller is responsible to ensure that the external
 * buffer is large enough to hold the copied data, the size of which is returned by
 * previous call to TCPConn::requestRecv.
 *
 * @param[out] buffer Pointer to the external buffer to copy the data into.
 * @param[out] size Pointer to a size_t, which will be set to the size of data copied
 *                  if not a NULL.
 * @return true if the operation is successful
 *         false if partial receive has occurred or the internal buffer is empty.
 */
bool TCPConn::retrieveRecvBuf(char *buffer, size_t *size)
{
    if (buffer == NULL || partial_receive || recv_data_size == 0) {
        return (false);
    }

    // Copy data to the external buffer
    std::memcpy(buffer, recv_buf, recv_retrieve_size);

	// Update external size variable to reflect the size.
	if (size != NULL)
    	*size = recv_data_size;

	// Update internal variables for subsequent calls
    recv_data_size -= recv_retrieve_size;
	// Move bytes in internal buffer to overwrite bytes copied to external buffer
	std::memmove(recv_buf, recv_buf + recv_retrieve_size, recv_data_size);
	recv_retrieve_size = 0;
    partial_receive = false;

    // Successful retrieval
    return (true);
}

short	TCPConn::getPollREvents() const throw()
{
	return (pollfd_struct->revents);
}

void	TCPConn::setPollREvents(short revents) throw()
{
	pollfd_struct->revents = revents;
}

short	TCPConn::getPollEvents() const throw()
{
	return (pollfd_struct->events);
}

void	TCPConn::setPollEvents(short events) throw()
{
	pollfd_struct->events = events;
}

void	TCPConn::setPollFdPtr(pollfd *ptr) throw()
{
	pollfd_struct = ptr;
}
