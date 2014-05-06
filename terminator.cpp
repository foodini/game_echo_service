#ifdef _MSC_VER

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "proxy_threads.h"
#include "proxy_sockets.h"

int client_listen_fd;

void *handle_connection(void *thread_arg)
{
    int thread_id = *(int*)thread_arg;
    char buf[65536];

    int client_fd = thread_args[thread_id];

    printf("%2d accepted connection\n", thread_id);
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(client_fd, &fds);

    int len;
    struct timeval timeout;
    for(;;)
    {
        timeout.tv_sec=10;
        timeout.tv_usec=0;

        len = read(client_fd,buf,sizeof(buf));
        if (len <= 0)
        {
            close(client_fd);
            break;
        }
        
        write(client_fd, buf, len);
    }
    printf("%2d disconnected\n", thread_id);
    completed_threads[thread_id] = 1;

    return NULL;
}

int main(int argc, char *argv[])
{
    init_sockets();
    init_threads();

    int listen_port = 7;
    if(argc == 2)
    {
        listen_port = atoi(argv[1]);
    }
    else
    {
        printf("usage: %s [local_port (default: %d)]\n", argv[0], listen_port);
    }

    printf("running server on port %d\n", listen_port);
    client_listen_fd = get_listen_fd(listen_port);
    
    for(;;)
    {
        int client_fd = accept(client_listen_fd, NULL, NULL);

        new_thread((void*)handle_connection, client_fd);
    }
}
