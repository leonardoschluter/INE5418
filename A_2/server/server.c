#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "server_socket.h"

int main() { 
    int server_sockfd = openServerSocket();    
    struct sockaddr_in* client_address = malloc(sizeof (struct soockaddr_in*));
    while(1) {
        
        int client_sockfd = acceptClientConnection(client_address, server_sockfd);
        
        //TODO: interpret client message and construct a response;
        printf("Client message: %s \n", readClientMessage(client_sockfd));

        char* hello = "Hello world";
        sendResponseClient(client_sockfd, hello);
        closeClientConnection(client_sockfd);       
    }
}