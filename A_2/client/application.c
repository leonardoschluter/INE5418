#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"

char* askForOperation(){
    char operation;
    int number1;
    int number2;
    printf("Informe uma operação dentre + , - , / , *\n");
    scanf ("%c", &operation);
    printf("Informe o primeiro operando ( deve ser número inteiro )\n");
    scanf ("%d", &number1);
    printf("Informe o segundo operando ( deve ser número inteiro )\n");
    scanf ("%d", &number2);
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
    char* calculation = askForOperation();
    sendMessage(calculation, sockfd);
    printf("char from server = %s\n", readMessage(sockfd));
    closeConnection(sockfd);
    exit(0);
}