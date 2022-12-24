#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <thread>
#include <fstream>
#include <orbis/Net.h>

#include "http_server.h"
#include "http_request.h"
#include "config.h"

#define SOCKET_BUFSIZ 1048576

#include <dbglogger.h>

std::mutex QueueLock;
std::queue<int> event_queue; // Events are Socket Numbers

class WebsiteHandler
{
public:
    WebsiteHandler()
    {
    }

    void Handle(int socket_num, Request *request)
    {
        if (request->GetRequestType() == HTTP_GET || request->GetRequestType() == HTTP_HEAD)
        {
            std::string filepath = Request::UrlDecode(request->GetPath());
            dbglogger_log("filepath=%s", filepath.c_str());
            if (filepath.empty() || !smbclient->IsConnected())
            {
                std::string str = "HTTP/1.1 404\r\n\r\n";
                sceNetSend(socket_num, str.c_str(), str.size(), 0);
                return;
            }
            int64_t file_size;
            int ret = smbclient->Size(filepath.c_str(), &file_size);
            if (ret == 0)
            {
                std::string str = "HTTP/1.1 500\r\n\r\nError getting filepath - " + filepath;
                sceNetSend(socket_num, str.c_str(), str.size(), 0);
                return;
            }
            std::string str = "HTTP/1.1 ";
            if (request->GetRequestType() == HTTP_GET)
                str = str + "200 OK\r\n";
            else
                str = str + "204 OK\r\n";
            str = str + "Content-Type: application/octet-stream;\r\n";
            std::string filename = filepath;
            size_t slash_pos = filename.find_last_of("/");
            if (slash_pos != std::string::npos)
            {
                filename = filename.substr(slash_pos + 1);
            }
            //str = str + "content-disposition: attachment; filename=" + filename + "\r\n";
            str = str + "content-length: " + std::to_string(file_size) + "\r\n\r\n";
            sceNetSend(socket_num, str.c_str(), str.size(), 0);

            if (request->GetRequestType() == HTTP_HEAD)
                return;

            smbclient->Copy(filepath.c_str(), socket_num);
        }
        else
        {
            std::string str = "HTTP/1.1 500\r\n\r\nUnsupported opeartion\r\n\r\n";
            sceNetSend(socket_num, str.c_str(), str.size(), 0);
        }
        return;
    }
};

WebsiteHandler website;

int HttpServer::NewSocket() // New socket for listen
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //! Fails
    if (server_fd < 0)
    {
        return -1;
    }
    return 0;
}

int HttpServer::BindAddress() // Bind address to socket
{
    int return_value = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    if (return_value < 0)
    {
        return -1;
    }
    return 0;
}

int HttpServer::StartListen(int k) // k is the max size of the queue
{
    int return_value = listen(server_fd, k);
    if (return_value < 0)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief  accepts new requests from server_fd,
 * @retval int, the value of the connection socket
 */
int HttpServer::AcceptConnection()
{
    linger lng = {0, 0};
    int on = 1;

    int fd = accept(server_fd, (struct sockaddr *)&address, &sizeof_address);
    if (fd < 0)
    {
        return -1;
    }

    int const size = SOCKET_BUFSIZ;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) == -1)
    {
        close(fd);
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) == -1)
    {
        close(fd);
        return -1;
    }

    return fd;
}

/**
 * @brief  Server Constructer
 * @param  internet_address: internet address
 * @param  port_number: port number, Default:80
 * @param  thread_count: Number Of Thread Count for a proccess
 * @retval
 */
HttpServer::HttpServer(int internet_address, int port_number, int thread_count) // 80 for http
{
    this->thread_count = thread_count;
    server_up = 0;
    sizeof_address = sizeof(address);
    address.sin_family = AF_INET; // Internet Based
    if (internet_address == 0)
        address.sin_addr.s_addr = INADDR_ANY; // accept any incoming
    else
        address.sin_addr.s_addr = internet_address;
    address.sin_port = htons(port_number);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    if (NewSocket() == -1)
        return;
    if (BindAddress() == -1)
    {
        close(server_fd);
        return;
    }
    if (StartListen() == -1)
    {
        close(server_fd);
        return;
    }
    server_up = 1;
}

HttpServer::~HttpServer()
{
    shutdown(server_fd, 2);
}

/**
 *
 */
void *HttpServer::ConnectionThread(void *argv)
{
    while (true)
    {
        int socket_num;
        QueueLock.lock();
        if (event_queue.empty() == false)
        {
            socket_num = event_queue.front();
            event_queue.pop();
            QueueLock.unlock();
        }
        else
        {
            QueueLock.unlock();
            continue;
        }

        char buffer[3072] = {
            0};
        memset(buffer, 0, 3072);
        dbglogger_log("before recv");
        int buffer_length = recv(socket_num, buffer, 3072, 0);
        if (buffer_length < 0)
        {
            perror("ERROR: Receiving Failure\n");
            return NULL;
        }
        dbglogger_log("after recv");

        Request request(buffer, buffer_length);

        dbglogger_log("after request");
        website.Handle(socket_num, &request);
        close(socket_num);
        dbglogger_log("close socket");
    }
    return NULL;
}

void *HttpServer::AcceptConnectionThread(void *argv)
{
    HttpServer *server = (HttpServer*)argv;
    while (1)
    {
        int socket_num = server->AcceptConnection();
        dbglogger_log("socket_num=%d", socket_num);
        QueueLock.lock();
        event_queue.push(socket_num);
        QueueLock.unlock();
    }
    return NULL;
}

void HttpServer::Start()
{
    if (server_up == 0)
    {
        std::cerr << "Server failed to start!\n";
        return;
    }

    pthread_t ptid[thread_count];
    for (int i = 0; i < thread_count; i++)
    {
        int return_value = pthread_create(&ptid[i], NULL, ConnectionThread, (void *)NULL);
        if (return_value < 0)
        {
            perror("ERROR: Couldn't create thread\n");
            return;
        }
    }

    pthread_t apid;
    int return_value = pthread_create(&apid, NULL, AcceptConnectionThread, (void *)this);
    if (return_value < 0)
    {
        perror("ERROR: Couldn't create thread\n");
        return;
    }
}