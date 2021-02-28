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
    int sockfd;
    socklen_t len;
    struct sockaddr_in address;
    int result;
    char buffer[1024];
    char* calculation = "#+#3#7\0";
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
    write(sockfd, calculation, sizeof(calculation));
    bzero(buffer, 1024);
    read(sockfd, buffer, sizeof(buffer));
    printf("char from server = %s\n", buffer);
    close(sockfd);
    exit(0);
}