// proxy.cpp : Defines the entry point for the console application.
//

#ifdef _MSC_VER
//You CANNOT use precompiled headers, or this will cause an issue on the endif.
#include "stdafx.h"
//You CANNOT use precompiled headers, or this will cause an issue on the endif.
#endif

#ifdef _MSC_VER
#include "proxy_threads.h"
#include "proxy_sockets.h"
#define read(sock,buf,len) recv(sock,buf,len,0)
#define write(sock,buf,len) send(sock,buf,len,0)
#define close closesocket
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
#include "proxy_threads.h"
#include "proxy_sockets.h"
#endif

#define MAX(a,b) (a > b ? a : b)
#define MAX4(a,b,c,d) MAX(MAX(a,b),MAX(c,d))


int client_listen_fd;
char * server_ip_addr_string = "127.0.0.1";
int server_port = 27050;
char * echo_ip_addr_string = "127.0.0.1";
int echo_port = 0;

void *handle_game_connection(void * tid_ptr)
{
    int thread_id = *(int*)tid_ptr;
    int client_fd = thread_args[thread_id];

    printf("%2d Accepted connection on socket %d...\n", thread_id, client_fd);

    int server_fd = get_remote_fd(server_ip_addr_string, server_port, thread_id);
    int client_to_server_echo_fd = 0;
    int server_to_client_echo_fd = 0;

    if(echo_port)
    {
        client_to_server_echo_fd = get_remote_fd(echo_ip_addr_string, echo_port, thread_id);
        server_to_client_echo_fd = get_remote_fd(echo_ip_addr_string, echo_port, thread_id);
    }

    if(server_fd < 0)
        fprintf(stderr, "%2d Cannot connect to game server.\n", thread_id);
    if(client_to_server_echo_fd < 0 || client_to_server_echo_fd < 0)
        fprintf(stderr, "%2d Cannot connect to echo server.\n", thread_id);
    if(server_fd < 0 || client_to_server_echo_fd < 0 || client_to_server_echo_fd < 0)
    {    
        completed_threads[thread_id] = 1;
        return NULL;
    }

    int numfds = 1 + MAX4(client_fd, server_fd, client_to_server_echo_fd, server_to_client_echo_fd);
    
    fd_set fds;
    struct timeval timeout;

    for(;;)
    {
        timeout.tv_sec=10;
        timeout.tv_usec=0;

        FD_ZERO(&fds);
        FD_SET(client_fd, &fds);
        FD_SET(server_fd, &fds);
        if(echo_port)
        {
            FD_SET(client_to_server_echo_fd, &fds);
            FD_SET(server_to_client_echo_fd, &fds);
        }

        int select_result = select(numfds, &fds, NULL, NULL, NULL);

        if(select_result == -1)
        {
            char buf[1000];
            fprintf(stderr, "SOCKET ERROR!!!!!\n");
            sprintf(buf, "select err: client_fd(%d), server_fd(%d)\n", client_fd, server_fd);
            sock_error(buf, 0);
            sock_error("A select() error has ocurred\n", 1);
            force_exit(client_fd, server_fd, thread_id);
            break;
        }
        else if(select_result == 0)
        {
            sock_error("Timeout\n", 0);
            continue;
        }
        //I'm not sure about my decision here.  It keeps the server/client lag balanced, but it does
        //mean that the net effect, from the perspective of the player, is 2x(echo RTT).
        if(echo_port)
        {
            if(FD_ISSET(client_fd, &fds) && forward(client_fd, client_to_server_echo_fd, thread_id) <= 0)
                    break;
            if(FD_ISSET(client_to_server_echo_fd, &fds) && forward(client_to_server_echo_fd, server_fd, thread_id) <= 0)
                    break;
            if(FD_ISSET(server_fd, &fds) && forward(server_fd, server_to_client_echo_fd, thread_id) <= 0)
                    break;
            if(FD_ISSET(server_to_client_echo_fd, &fds) && forward(server_to_client_echo_fd, client_fd, thread_id) <= 0)
                    break;
        }
        else
        {
            if(FD_ISSET(client_fd, &fds) && forward(client_fd, server_fd, thread_id) <= 0)
                    break;
            if(FD_ISSET(server_fd, &fds) && forward(server_fd, client_fd, thread_id) <= 0)
                    break;
        }
    }
    close(client_fd);
    close(server_fd);
    if(echo_port)
    {
        close(client_to_server_echo_fd);
        close(server_to_client_echo_fd);
    }

    completed_threads[thread_id] = 1;
    return NULL;
}

void *handle_policy_connections(void * tid_ptr)
{
    int thread_id = *(int*)tid_ptr;
    int port = thread_args[thread_id];

    char * policy = "<?xml version='1.0'?>\n<cross-domain-policy>\n\t<allow-access-from domain=\"*\" to-ports=\"*\" />\n</cross-domain-policy>\n";
    char policy_client_request[65535];
    int len = strlen(policy);

    int proxy_client_listen_fd = get_listen_fd(port);

    for(;;)
    {
        printf("Listening for new policy connections...\n");
        int policy_client_fd = accept(proxy_client_listen_fd, NULL, NULL);

        if(0 > policy_client_fd)
        {
            sock_error("error accepting policy connection", 0);
            continue;
        }

        printf("accepted policy connection.\n");

        read(policy_client_fd, policy_client_request, 65536);
        printf("received policy request: %s\n", policy_client_request);
        write(policy_client_fd, policy, len);
        close(policy_client_fd);
    }
}


#define parse_str_opt(cmd,store) if(!strcmp(argv[i], cmd)){i++; store=argv[i]; continue;}
#define parse_int_opt(cmd,store) if(!strcmp(argv[i], cmd)){i++; store=atoi(argv[i]); continue;}

void usage(char *argv[])
{
    fprintf(stderr, "usage: %s options\n", argv[0]);
    fprintf(stderr, "\t[--gip game_server_address] - game server dotted-decimal ip.  Default: 127.0.0.1\n");
    fprintf(stderr, "\t[--gpt game_server_port]    - game server port number.        Default: 27050\n");
    fprintf(stderr, "\t[--ppt local_port]          - local port number for proxy.    Default: 27052\n");
    fprintf(stderr, "\t[--eip echo_server_address] - echo server dotted-decimal ip.  Default: 127.0.0.1\n");
    fprintf(stderr, "\t[--ept echo_server_port]    - echo server port number.        Default: no echoing\n");
    exit(1);
}

int main(int argc, char * argv[])
{
    init_sockets();
    init_threads();

    int proxy_port = 27052;

    if(argc == 1)
    {
        printf("Using defaults.  Call with --help for options.\n");
    }

    for(int i=1; i<argc; i++)
    {
        if(!strncmp("-h", argv[i], 2) || !strncmp("--h", argv[i], 3))
            usage(argv);
        parse_str_opt("--gip", server_ip_addr_string);
        parse_int_opt("--gpt", server_port);
        parse_int_opt("--ppt", proxy_port);
        parse_str_opt("--eip", echo_ip_addr_string);
        parse_int_opt("--ept", echo_port);

        fprintf(stderr, "Cannot parse option, %s\n", argv[i]);
    }


    // This is only useful if you're using Unity. It will be made optional in a later changelist.
    int policy_port = proxy_port + 1;
    new_thread((void*)handle_policy_connections, policy_port);
    
    client_listen_fd = get_listen_fd(proxy_port);

    for(;;)
    {
        int client_fd = accept(client_listen_fd, NULL, NULL);

        if(client_fd <= 0)
        {
            sock_error("error accept failed", 0);
            sleep(10);
            continue;
        }

        new_thread((void*)handle_game_connection, client_fd);
    }
}
