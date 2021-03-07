#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

char * close_command = "sair";

char* askForOperation(){
    char operation = ' ';
    int number1 = 0;
    int number2 = 0;
    printf("Informe uma operação dentre + , - , / , * ou 's' para sair da calculadora\n");
    scanf(" %c", &operation);

    if(operation == 's'){
        return close_command;
    }

    printf("Informe o primeiro operando ( deve ser número inteiro )\n");
    scanf("%d", &number1);
    printf("Informe o segundo operando ( deve ser número inteiro )\n");
    scanf("%d", &number2);
    char *buffer = malloc(1024);
    buffer[0]=operation;
    char* number1str = malloc(128);
    char* number2str = malloc(128);
    sprintf(number1str, "#%d", number1);
    sprintf(number2str, "#%d", number2);
    strcat(buffer, number1str);
    strcat(buffer, number2str);
    printf("Entrada foi: %s\n", buffer);
    return buffer;
}

int main() {
    int sockfd = connectToServer();
    while(1){
        char* calculation = askForOperation();
        sendMessage(calculation, sockfd);
        char * result = readMessage(sockfd);
        if(*result == *close_command){
            closeConnection(sockfd);
            printf("Bye Bye !!!");
            exit(0);
        }
        printf("calculation result = %s\n", result);
    }    
    exit(0);
}