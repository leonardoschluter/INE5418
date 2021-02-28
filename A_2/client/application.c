#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main() {
    int sockfd = connectToServer();
    char* calculation = "#+#3#7\0";
    sendMessage(calculation, sockfd);
    char* buffer = readMessage(sockfd);
    printf("char from server = %s\n", buffer);
    closeConnection(sockfd);
    exit(0);
}