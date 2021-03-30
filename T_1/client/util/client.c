#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int connectToServer(){
    int sockfd;
    socklen_t len;
    struct sockaddr_in address;
    int result;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = 8080;
    len = sizeof(address);
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: client1");
        exit(1);
    }
    return sockfd;
}

void closeConnection(int sockfd){
    close(sockfd);
}


void sendMessage(char* msg, int sockfd){   
    printf("next command -> %s\n", msg);
    write(sockfd, msg, sizeof(char*)*(128));
}

char* readMessage(int sockfd){
    char *buffer = malloc(2048);
    bzero(buffer, 2048);
    read(sockfd, buffer, sizeof(buffer));
    return buffer;

}
