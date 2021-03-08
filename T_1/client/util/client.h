#ifndef CLIENT_H_   /* Include guard */
#define CLIENT_H_

char* readMessage(int sockfd);

void sendMessage(char* msg, int sockfd);

int connectToServer();

void closeConnection(int sockfd);
#endif // CLIENT_H_