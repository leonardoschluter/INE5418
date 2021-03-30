#ifndef CLIENT_H_   /* Include guard */
#define CLIENT_H_

struct connection_n;

typedef struct connection_n{
	int sock;
	struct sockaddr_in address;
	int addr_len;
} connection_node;

char* readMessage(int sockfd);

void sendMessage(char* msg, int sockfd);

struct connection_n * connectToServer(char* server_addr, short unsigned int server_port);

void closeConnection(int sockfd);
#endif // CLIENT_H_