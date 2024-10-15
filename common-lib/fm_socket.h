#ifndef FM_SOCKET_H
#define FM_SOCKET_H

#ifdef __cplusplus
extern "C"
{
#endif

void setnonblocking(int fd);

int fm_create_tcp_service(const char* service_addr, short service_port);

int fm_wait_client(int service_fd);

int fm_rcv_msg(int fd, void* msg, int msg_len);

int fm_snd_msg(int fd, void* msg, int msg_len);

#ifdef __cplusplus
}
#endif
#endif