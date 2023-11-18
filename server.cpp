#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int	main(void)
{
	int			fd_sock_listen;
	int			fd_sock_client;
	//int			yes;
	sockaddr_in	addr_listen;
	sockaddr_in	addr_client;
	socklen_t	len_addr_client;
	int			bytes_recv;
	char		buf[4096] = {0};

	//yes = 1;
	fd_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	//fd_sock_listen = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	//setsockopt(fd_sock_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	addr_listen.sin_family = AF_INET;
	addr_listen.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr_listen.sin_port = htons(54000);
	bind(fd_sock_listen, (sockaddr *) &addr_listen, sizeof(addr_listen));
	std::cout << "Listening" << std::endl;
	listen(fd_sock_listen, SOMAXCONN);
	fd_sock_client = accept(fd_sock_listen, (sockaddr *) &addr_client, &len_addr_client);
	std::cout << "Client connected : " << fd_sock_client << std::endl;
	while (true)
	{
		bytes_recv = recv(fd_sock_client, buf, 4096, 0);
		if (bytes_recv == 0)
		{
			std::cout << "Client disconnected" << std::endl;
			break ;
		}
		else if (bytes_recv > 0)
		{
			std::cout << std::string(buf).substr(0, bytes_recv) << std::endl;
			send(fd_sock_client, buf, bytes_recv, 0);
		}
	}
	close(fd_sock_client);
	close(fd_sock_listen);
}
