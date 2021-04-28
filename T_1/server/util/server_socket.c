#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void closeClientConnection(int client_sockfd){
    close(client_sockfd);
}

int openServerSocket(){

    int server_sockfd;
    socklen_t server_len;
    struct sockaddr_in server_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = 8080;
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len)<0){
        perror("Binding failed");
    }
    listen(server_sockfd, 5);  
    return server_sockfd;

}

int acceptClientConnection(struct sockaddr_in *client_address, int server_sockfd ){
    printf("server waiting for client connection \n");
    socklen_t client_len = sizeof(*client_address);
    return accept(server_sockfd,(struct sockaddr *)client_address, &client_len);
     
}

char * readClientMessage(int client_sockfd){
    char * buffer = malloc( sizeof(char*)*(128));
    bzero(buffer, sizeof(char*)*(128));
    ssize_t result = read(client_sockfd, buffer, sizeof(char*)*(128));
    printf("result do read -> %ld\n", result);
    printf("buffer da msg -> %s\n", buffer);
    return buffer;
}

void sendResponseClient(int client_sockfd, char* msg){ 
    printf("writing %s\n", msg);
    write(client_sockfd, msg,  sizeof(char*)*(128));
}

void sendResponseAndClose(int sock, char * response){
	sendResponseClient(sock, response);
	printf("server: op response %s\n", response);
	close(sock);
}