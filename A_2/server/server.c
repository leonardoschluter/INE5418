#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "server_socket.h"
#include <string.h>
char * close_command = "sair";

void doCalculation(char * message, int client_sockfd){
    char * operator_str = strsep(&message, "#");
    char * number1_str = strsep(&message, "#");
    char * number2_str = strsep(&message, "#");

    int number1 = atoi(number1_str);
    int number2 = atoi(number2_str);

    char validator1[16];
    char validator2[16];
    sprintf(validator1, "%d", number1);
    sprintf(validator2, "%d", number2);

    if(*validator1 != *number1_str || *validator2 != *number2_str){   
        char* errorMsg = "Invalid operands";   
        printf("error: %s \n", errorMsg);      
        sendResponseClient(client_sockfd, errorMsg);
    }else{
        int result = 0;
        if(*operator_str == '*'){
            result = number1 * number2;                
        }else if(*operator_str == '/' && number2 != 0){
            result = number1 / number2;
        }else if(*operator_str == '+'){
            result = number1 + number2;                
        }else if(*operator_str == '-'){
            result = number1 - number2;                
        }else{
            char* errorMsg = "Invalid operator";
            printf("error: %s \n", errorMsg);    
            sendResponseClient(client_sockfd, errorMsg);
        }
        char result_str[20];
        sprintf(result_str, "%d", result);
        printf("result_str: %s \n", result_str);
        sendResponseClient(client_sockfd, result_str);
    } 
}

int calculatorLoop(int server_sockfd ,struct sockaddr_in* client_address){
    int client_sockfd = acceptClientConnection(client_address, server_sockfd);
    while(1) {        
        char * message = readClientMessage(client_sockfd);        
        if(*message == *close_command){
            sendResponseClient(client_sockfd, close_command);
            closeClientConnection(client_sockfd);
            return 0;
        }else{
            doCalculation(message, client_sockfd);
        }    
    }
}
int main() { 
    int server_sockfd = openServerSocket();    
    struct sockaddr_in* client_address = malloc(sizeof (struct soockaddr_in*));
    while(1){
        calculatorLoop(server_sockfd, client_address);
    }
}