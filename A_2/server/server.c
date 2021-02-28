#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int main() {
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    char buffer[1024]; 
    char* hello = "Hello world\0";
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = 8080;
    server_len = sizeof(server_address);
    if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len)<0){
        perror("Binding failed");
    }
    listen(server_sockfd, 5);   
    while(1) {
        printf("server waiting for calculation \n");
        client_len = sizeof(client_address);
        printf("server: empty buffer = %s\n", buffer);
        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address, &client_len);
        
        printf("server: empty buffer = %s\n", buffer);
        bzero(buffer, 1024);
        printf("server: empty buffer = %s\n", buffer);
        int res = read(client_sockfd, buffer, sizeof(buffer));
        printf("char from server = %s\n", buffer);
        write(client_sockfd, hello, sizeof(hello));
        close(client_sockfd);       
    }
}