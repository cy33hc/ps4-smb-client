#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

class HttpServer
{
public:
	HttpServer(int internet_address, int port_number = 80, int thread_count = 10);
	~HttpServer();
	static void *ConnectionThread(void *argv);
	static void *AcceptConnectionThread(void *argv);
	void Start();

private:
	int server_fd;
	socklen_t sizeof_address;
	int thread_count;
	struct sockaddr_in address;
	int server_up;

	int NewSocket();
	int BindAddress();
	int AcceptConnection();
	int StartListen(int queue_size = 100);
};

#endif