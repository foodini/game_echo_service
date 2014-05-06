#ifndef _PROXY_SOCKETS_H
#define _PROXY_SOCKETS_H

#ifdef _MSC_VER
#include <windows.h>
#include <strsafe.h>
#define read(sock,buf,len) recv(sock,buf,len,0)
#define write(sock,buf,len) send(sock,buf,len,0)
#define close closesocket
#ifndef sleep
#define sleep(t) Sleep(1000*t)
#endif
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#endif

void sock_error(char * msg, int fatal);
int get_listen_fd(int port);
int get_remote_fd(char * address, int port, int thread_id);
void force_exit(int client_fd, int server_fd, int thread_id);
int forward(int read_fd, int write_fd, int thread_id);
void init_sockets();

#endif
