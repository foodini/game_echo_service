#include "proxy_sockets.h"

void sock_error(char * msg, int fatal)
{
#ifdef _MSC_VER
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)msg)+40)*sizeof(TCHAR)); 
    size_t msg_size = strlen(msg) + 1;
    wchar_t * wmsg = new wchar_t[msg_size];
    mbstowcs(wmsg, msg, msg_size);
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("(%s) - failed with error %d: %s"), 
        wmsg, dw, lpMsgBuf);
    delete[] wmsg;
    //MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
    msg_size = wcstombs(NULL, (LPTSTR)lpDisplayBuf, 0);
    char * displaybuf = new char[msg_size + 1];
    wcstombs(displaybuf, (LPTSTR)lpDisplayBuf, msg_size + 1);
    fprintf(stderr, "%s\n", displaybuf);
    fflush(stderr);
    delete[] displaybuf;

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

#endif
    perror(msg);
    if(fatal)
    {
        sleep(10);
        exit(fatal);
    }
}

int get_listen_fd(int port)
{
    struct sockaddr_in recv_addr;
    int client_listen_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(client_listen_fd == -1)
        sock_error("can not create socket", 1);

    memset(&recv_addr, 0, sizeof(recv_addr));

    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(port);
    recv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(client_listen_fd,(struct sockaddr *)&recv_addr, sizeof(recv_addr)) == -1)
        sock_error("error bind failed", 1);

    if(listen(client_listen_fd, 10) == -1)
        sock_error("error listen failed", 1);

    return client_listen_fd;
}

int get_remote_fd(char * address, int port, int thread_id)
{
    printf ("%2d Setting up socket to talk to %s:%d...\n", thread_id, address, port);

    struct sockaddr_in server_addr;
    int server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_fd <= 0)
        sock_error("cannot create socket", 1);
 
    memset(&server_addr, 0, sizeof(server_addr));
 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
#ifdef _MSC_VER
    hostent *host;
    if((host = gethostbyname(address)) == 0)
        sock_error("Cannot lookup hostname.", 1);

    server_addr.sin_addr = *((in_addr*)host->h_addr);
#else
    int inet_pton_result = inet_pton(AF_INET, address, &server_addr.sin_addr);
 
    if (inet_pton_result < 0)
        sock_error("error: first parameter is not a valid address family", 1);
    else if (inet_pton_result == 0)
        sock_error("char string (second parameter does not contain valid ipaddress)", 1);
 
#endif
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        sock_error("connect failed", 0);
        return -1;
    }

    return server_fd;
}

void force_exit(int client_fd, int server_fd, int thread_id)
{
    close(client_fd);
    close(server_fd);
    printf("%2d Connection closed\n", thread_id);
}


char buf[65536];
int forward(int read_fd, int write_fd, int thread_id)
{
    int len = read(read_fd,buf,sizeof(buf));
    if (len > 0)
        write(write_fd,buf,len);
    else
        force_exit(read_fd, write_fd, thread_id);

    return len;
}

void init_sockets()
{
#if _MSC_VER
    WSADATA wsaData;
    int startup = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(startup)
        sock_error("WSAStartup", 1);
#endif
}
