#ifndef SERVER_SOCKET_H_   /* Include guard */
#define SERVER_SOCKET_H_

struct sockaddr_in;

void closeClientConnection(int client_sockfd);

int openServerSocket();

int acceptClientConnection(struct sockaddr_in *client_address, int server_sockfd );

char * readClientMessage(int client_sockfd);

void sendResponseClient(int client_sockfd, char* msg);

void sendResponseAndClose(int sock, char * response);


#endif // SERVER_SOCKET_H_