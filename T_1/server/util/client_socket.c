#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "client_socket.h"



struct connection_n * connectToServer(char* server_addr, short unsigned int server_port){
    int sockfd;
    socklen_t len;
    struct sockaddr_in address;
    int result;
    struct connection_n * node = malloc(sizeof(struct connection_n));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_addr);
    address.sin_port = server_port;
    len = sizeof(address);
	printf("server: node addr %s & port %d", server_addr, server_port);
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1) {
        perror("oops: client1");
        exit(1);
    }
    node->sock = sockfd;
    node->address = address;
    node->addr_len = len;
    return node;
}

void closeConnection(int sockfd){
    close(sockfd);
}

void sendMessage(char* msg, int sockfd){   
    printf("next command -> %s\n", msg);
    write(sockfd, msg,  sizeof(char*)*(128));
}

char* readMessage(int sockfd){
    char * buffer = malloc( sizeof(char*)*(128));
    bzero(buffer, sizeof(char*)*(128));
    ssize_t result = read(sockfd, buffer, sizeof(char*)*(128));
    printf("result do read -> %ld\n", result);
    printf("buffer da msg -> %s\n", buffer);
    return buffer;

}
