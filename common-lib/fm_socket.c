#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include "data_handle.h"

void setnonblocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

int fm_create_tcp_service(const char* service_addr, short service_port)
{
    struct sockaddr_in server_addr;
    int ret = 0;
    int server_fd = -1;

    memset(&server_addr, 0, sizeof(server_addr));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){
        fm_error("Failed to create a service connecting to the gnb, socket error: %s", strerror(errno));
        goto socket_error;
    }

    server_addr.sin_addr.s_addr = inet_addr(service_addr); //设置ip地址

    server_addr.sin_family = AF_INET; //设置tcp协议族
	server_addr.sin_port = htons(service_port); //设置端口号

    ret = bind(server_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
    if(ret != 0){
        fm_error("Bind error when create amf service connecting to gnb, %s", strerror(errno));
        goto socket_error;
    }

    ret = listen(server_fd, 10);
    if(ret != 0){
        fm_error("listen error when create amf service connecting to gnb, %s", strerror(errno));
        goto socket_error;
    }

    return server_fd;

socket_error:
    if(server_fd != -1) close(server_fd);

    return FM_ERROR;
}

int fm_wait_client(int service_fd)
{
    struct sockaddr client_addr;
	socklen_t clinent_sock_len = sizeof(client_addr);
    //是否阻塞由调用者决定
    int client_fd = accept(service_fd, &client_addr, &clinent_sock_len);

    if(client_fd == -1){
        fm_error("accept error, %s", strerror(errno));
        close(service_fd);
        return FM_ERROR;
    }

    fm_info("Connected a client: %s: %d", inet_ntoa(((struct sockaddr_in*)&client_addr)->sin_addr), 
                        ntohs(((struct sockaddr_in*)&client_addr)->sin_port));

    return client_fd;
}

int fm_rcv_msg(int fd, void* msg, int msg_len)
{
    int ret = 0;

    ret = recv(fd, msg, msg_len, 0);
    if(ret == 0){
        fm_info("FM client has closed socket");
        close(fd);
        return FM_ERROR;
    }else if(ret == -1){
        if(errno == EINTR || errno == EAGAIN){
            fm_debug("Message reception completed");
            return FM_OK;
        }else{
            fm_warn("FM service error occurred, error: %s", strerror(errno));
            close(fd);
            return FM_ERROR;
        }
    }else{
        return ret;
    }

    return FM_OK;
}

int fm_snd_msg(int fd, void* msg, int msg_len)
{
    int ret = 0;

    ret = send(fd, msg, msg_len, 0);
    if(ret != sizeof(msg)){
        fm_error("Failed to send msg, %s\n", strerror(errno));
        return FM_ERROR;
    }

    return ret;
}

