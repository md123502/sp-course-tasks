#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define BACKLOG_LENGTH 5
#define BUFFER_SIZE 1000
#define PORT 51000

enum ERRORS {
    SOCKET_ERROR = 1,
    BIND_ERROR,
    LISTEN_ERROR,
    ACCEPT_ERROR,
    READ_ERROR,
    WRITE_ERROR,
    FORK_ERROR
};

int handle_client(int sockfd) {
    char line[BUFFER_SIZE];
    memset(line, 0, BUFFER_SIZE);
    int bytes_rw = 0;
    while (0 < (bytes_rw = read(sockfd, line, BUFFER_SIZE - 1))) {
        if (0 > (bytes_rw = write(sockfd, line, bytes_rw))) {
            return WRITE_ERROR;
        }
    }
    if(0 > bytes_rw){
        return READ_ERROR;
    }
    return 0;
}

int main() {
    int sockfd = 0;
    errno = 0;
    if (0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0))){
        perror("socket() error");
        return SOCKET_ERROR;
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family= AF_INET;
    servaddr.sin_port= htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (0 > bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) {
        perror("bind() error");
        close(sockfd);
        return BIND_ERROR;
    }
    if (0 > listen(sockfd, BACKLOG_LENGTH)){
        perror("listen() error");
        close(sockfd);
        return LISTEN_ERROR;
    }
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int newsockfd = 0;
    while (true) {
        clilen = sizeof(cliaddr);
        if(0 > (newsockfd = 
                accept(sockfd, (struct sockaddr *) &cliaddr, &clilen))) {
            perror("accept() error");
            close(sockfd);
            return ACCEPT_ERROR;
        }
        printf("Accepted connection\n");
        pid_t fork_result = 0;
        if (0 == (fork_result = fork())) {
            close(sockfd);
            int result = 0;
            if ((result = handle_client(newsockfd))) {
                if (READ_ERROR == result) {
                    perror("read() error");
                }
                if (WRITE_ERROR == result) {
                    perror("write() error");
                }
            }
            close(newsockfd);
            return result;
        }
        if (0 > fork_result) {
            perror("fork() error");
            close(sockfd);
            close(newsockfd);
            return FORK_ERROR;
        }
        close(newsockfd);
    }
}
