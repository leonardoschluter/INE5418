#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/client.h"

char * close_command = "sair";

char* askForOperation(){
    char operation = ' ';
    int addr = 0;
    unsigned int size = 0;
    char* data;
    printf("Informe uma operação dentre 1 - para escrever, 2 - para ler ou \n 's' para sair da memória virtual\n");
    scanf(" %c", &operation);

    if(operation == 's'){
        return close_command;
    }

    printf("Informe o endereço\n");
    scanf("%d", &addr);
    printf("Informe o tamanho\n");
    scanf("%d", &size);
    char *buffer = malloc(1024);
    buffer[0]=operation;
    char* addrStr = malloc(32);
    char* sizeStr = malloc(32);
    sprintf(addrStr, "#%d", addr);
    sprintf(sizeStr, "#%d", size);
    strcat(buffer, addrStr);
    strcat(buffer, sizeStr);


    if(operation=='1'){
        data = malloc(1024);
        printf("Informe os dados\n");
        scanf("%s", data);
        strcat(buffer, "#");
        strcat(buffer, data);
    }
    
    printf("Entrada foi: %s\n", buffer);
    return buffer;
}

int main() {
    while(1){
        int sockfd = connectToServer();
        char* calculation = askForOperation();
        sendMessage(calculation, sockfd);
        char * result = readMessage(sockfd);
        printf("calculation result = %s\n", result);
        closeConnection(sockfd);
    }    
    exit(0);
}