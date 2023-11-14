#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define BUFFSIZE 2048
#define DEFAULT_PORT 16555
#define MAXLINK 2048
int sockfd,connfd;
int client_idx = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char hello_str[] = "\033[EYou have connected the lrm's Server, type quit to exit\n";
void stopServerRunning(int p)
{
    close(sockfd);
    printf("Close Server\n");
    exit(0);
}
int receive_line(int sockfd, char *buffer, int max_length) {
    int total_received = 0;
    char *buffer_ptr = buffer;

    while (total_received < max_length - 1) {
        // 从套接字接收一个字符
        ssize_t result = recv(sockfd, buffer_ptr, 1, 0);

        if (result > 0) {
            // 成功接收一个字符
            if (*buffer_ptr == '\r') 
                continue;
            else if (*buffer_ptr == '\n')
            {
                buffer_ptr++;
                total_received++;
                break;
            }
            // 移动指针和累积接收的字节数
            buffer_ptr++;
            total_received++;
        } else if (result == 0) {
            // 连接关闭
            return -1;
        } else {
            // 接收数据出错
            perror("recv");
            return -1;
        }
    }

    // 在末尾添加 null 终止符
    *buffer_ptr = '\0';

    return total_received;
}
struct my_args{
    int connfd;
    int myId;
};
void* do_response(void* args){
    struct my_args *p = (struct my_args*)args;
    int connfd = p->connfd;
    int myId = p->myId;
    char buff[BUFFSIZE];
    send(connfd,hello_str,strlen(hello_str),0);
    bzero(buff,BUFFSIZE);
    while (true)
    {
        if(receive_line(connfd, buff, BUFFSIZE) == -1) 
            break;
        if (strcmp(buff,"quit\n") == 0) 
            break;
        printf("Recv client%d: %s",myId, buff);
        send(connfd,buff,strlen(buff),0);
        bzero(buff,BUFFSIZE);
    }
    printf("Client%d leaves me\n\n",myId);
    close(connfd);
    return NULL;
}

int main()
{
    struct sockaddr_in server_addr;
    char buff[BUFFSIZE];
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1){
        printf("Create socket error(%d): %s\n",errno,strerror(errno));
        return -1;
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(DEFAULT_PORT);

    if(-1 == bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        printf("Bind error(%d):%s\n",errno,strerror(errno));
        return -1;
    }

    if (-1 == listen(sockfd,MAXLINK)){
        printf("Listen error(%d): %s\n", errno, strerror(errno));
        return -1; 
    }

    printf("Listening...\n\n");
    while (true)
    {
        signal(SIGINT,stopServerRunning);
        struct sockaddr_in client_addr;
        char client_ip[INET_ADDRSTRLEN];
        socklen_t client_addr_len = sizeof(client_addr);
        connfd = accept(sockfd,(struct sockaddr*)&client_addr,&client_addr_len);
        if (-1 == connfd){
            printf("Accept error(%d): %s\n", errno, strerror(errno));
            return -1;
        }
        pthread_mutex_lock(&mutex);
        int myId = client_idx++;
        pthread_mutex_unlock(&mutex);
        printf("Accept client%d, ip:%s\t, port:%d\n",myId,
            inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,client_ip,sizeof(client_ip)),
            ntohs(client_addr.sin_port));
        pthread_t myThread;
        struct my_args args;
        args.connfd = connfd;
        args.myId = myId;
        pthread_create(&myThread,NULL,do_response,&args);
        // send(connfd,hello_str,strlen(hello_str),0);
        // bzero(buff,BUFFSIZE);
        // while (true)
        // {
        //     if(receive_line(connfd, buff, BUFFSIZE) == -1) 
        //         break;
        //     if (strcmp(buff,"quit\n") == 0) 
        //         break;
        //     printf("Recv client%d: %s",myId, buff);
        //     send(connfd,buff,strlen(buff),0);
        //     bzero(buff,BUFFSIZE);
        // }
        // printf("Client%d leaves me\n\n",myId);
        // close(connfd);
    }
    return 0;
}