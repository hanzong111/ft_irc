#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <list>
#include <poll.h>
#include <fcntl.h>
#include <queue>

#define SEND_BUFFER_SIZE 4096
#define RECV_BUFFER_SIZE 4096

typedef struct	SendOp
{
	int			fd;
	std::string data;
} t_SendOp;

void	add_fd_to_vect(std::vector<pollfd> &vect, int fd, short events)
{
	pollfd	tmp;

	tmp.fd = fd;
	tmp.events = events;
	tmp.revents = 0;
	vect.push_back(tmp);
}

void	add_op_to_queue(std::deque<t_SendOp> &queue, int fd, const std::string &data)
{
	t_SendOp	tmp;

	tmp.fd = fd;
	tmp.data = data;
	queue.push_front(tmp);
}

int	main(void)
{
	int						fd_sock_listen;
	int						fd_sock_client;
	//int						yes;
	sockaddr_in				addr_listen;
	sockaddr_in				addr_client;
	socklen_t				len_addr_client;
	int						bytes_recv;
	char					recv_buf[RECV_BUFFER_SIZE + 1] = {0};
	std::string				recv_string;
	std::vector<pollfd>		vect_fds;
	std::deque<t_SendOp>	queue_send;

	//yes = 1;
	//fd_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	fd_sock_listen = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	add_fd_to_vect(vect_fds, fd_sock_listen, POLLIN);
	//setsockopt(fd_sock_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	addr_listen.sin_family = AF_INET;
	addr_listen.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr_listen.sin_port = htons(54000);
	bind(fd_sock_listen, (sockaddr *) &addr_listen, sizeof(addr_listen));
	std::cout << "Listening" << std::endl;
	listen(fd_sock_listen, SOMAXCONN);
	while (true)
	{
		poll(vect_fds.data(), vect_fds.size(), -12);
		// Handle recv
		for (size_t i = 0; i < vect_fds.size(); i++)
		{
			if (vect_fds[i].fd == fd_sock_listen && vect_fds[i].revents & POLLIN) // listener port
			{
				fd_sock_client = accept(fd_sock_listen, (sockaddr *) &addr_client, &len_addr_client);
				if (fd_sock_client == -1)
					perror("Connection");
				else
				{
					int	size_buf;

					fcntl(fd_sock_client, F_SETFL, O_NONBLOCK);
					std::cout << "Client connected : " << inet_ntoa(addr_client.sin_addr) << ":"
						<< ntohs(addr_client.sin_port) << std::endl;
					size_buf = SEND_BUFFER_SIZE;
					setsockopt(fd_sock_client, IPPROTO_TCP, SO_SNDBUF, &size_buf, sizeof(size_buf));
					size_buf = RECV_BUFFER_SIZE;
					setsockopt(fd_sock_client, IPPROTO_TCP, SO_RCVBUF, &size_buf, sizeof(size_buf));
					add_fd_to_vect(vect_fds, fd_sock_client, POLLIN);
				}
				vect_fds[i].revents &= ~POLLIN;
			}
			else if (vect_fds[i].fd != fd_sock_listen)
			{
				if (vect_fds[i].revents & POLLIN)
				{
					while ((bytes_recv = recv(vect_fds[i].fd, recv_buf, RECV_BUFFER_SIZE, 0)) == RECV_BUFFER_SIZE)
						recv_string.append(std::string(recv_buf).substr(0, bytes_recv));
					if (bytes_recv > 0)
					{
						recv_string.append(std::string(recv_buf).substr(0, bytes_recv));
						for (size_t j = 0; j < vect_fds.size(); j++)
						{
							if (vect_fds[j].fd != fd_sock_listen && j != i)
							{
								add_op_to_queue(queue_send, vect_fds[j].fd, recv_string);
								vect_fds[j].events |= POLLOUT;
								vect_fds[j].revents &= ~POLLIN;
							}
						}
					}
					else if (bytes_recv == 0)
					{
						std::cout << "Client disconnected" << std::endl;
						close(vect_fds[i].fd);
						vect_fds.erase(vect_fds.begin() + i);
						break ;
					}
				}
			}
		}
		// Handle send
		size_t	n_items = queue_send.size();
		for (size_t i = 0; i < n_items; i++)
		{
			for (std::vector<pollfd>::iterator it_v = vect_fds.begin(); it_v < vect_fds.end(); it_v++)
			{
				if (queue_send.front().fd == it_v->fd)
				{
					bool	is_send_complete = 0;

					if (it_v->revents & POLLOUT)
					{
						while (!queue_send.front().data.empty())
						{
							ssize_t sentBytes = send(queue_send.front().fd, queue_send.front().data.c_str(),
									queue_send.front().data.size(), 0);
							if (sentBytes > 0)
							{
								// Handle successful send
								if (queue_send.front().data.size() > (size_t) sentBytes)
									queue_send.front().data = queue_send.front().data.substr(sentBytes);
								else
									queue_send.front().data.clear();
							}
							else if (sentBytes == -1)
							{
								it_v->events |= POLLOUT;
								it_v->revents &= ~POLLOUT;
								is_send_complete = 0;
								break ;  // Exit the loop on error
							}
						}
						if (queue_send.front().data.empty()) // If send is completed
						{
							is_send_complete = 1;
							it_v->events &= ~POLLOUT;
						}
					}
					if (is_send_complete)
						queue_send.pop_front();
					else
					{
						const t_SendOp tmp = queue_send.front();
						queue_send.pop_front();
						queue_send.push_back(tmp);
					}
					break ;
				}
			}
		}
	}
	close(fd_sock_client);
	close(fd_sock_listen);
}
