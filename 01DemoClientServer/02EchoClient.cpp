#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFSIZE 2048
#define SERVER_IP "120.46.37.84"    // 指定服务端的IP，记得修改为你的服务端所在的ip
#define SERVER_PORT 16555            // 指定服务端的port
int main()
{
    struct sockaddr_in server_addr;
    char buff[BUFFSIZE];
    int sockfd;
    sockfd = socket(AF_INET,SOCK_STREAM,0);

    if (-1 == sockfd)
    {
        printf("Create socket error(%d): %s\n", errno, strerror(errno));
        return -1;
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);
    if (-1 == connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Connect error(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    printf("Connected to Sercer,Please input: ");
    scanf("%s", buff);
    send(sockfd, buff, strlen(buff), 0);
    bzero(buff, sizeof(buff));
    recv(sockfd, buff, BUFFSIZE - 1, 0);
    printf("Recv: %s\n", buff);
    close(sockfd);
}