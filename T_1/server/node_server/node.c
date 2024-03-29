#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h>  
#include <unistd.h> 
#include <string.h> 
#include <sys/socket.h>
#include <linux/in.h>

#include <ctype.h>
#include "memory.h"
#include "../util/server_socket.h"
int N = 10;

/*
*
* ###### NODE METHODS ######
*
*/

typedef struct{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

typedef struct{
    int code;
	int addr;
    unsigned int size;
    char* data;
} operation_t;

operation_t * defineOperation(char* msg){	
	operation_t * result = (operation_t *)malloc(sizeof(operation_t));
	
    printf("node: defineOperation da msg -> -%c-\n", msg[0]);
    char * code = strsep(&msg, "#");
	printf("node: operation code -%s- \n", code);
	if( *code != '1' && *code != '2'){
		return NULL;
	}
	result->code = atoi(code);
    result->addr = atoi(strsep(&msg, "#"));
    char * str_size = strsep(&msg, "#");

	if(*code == '1'){
		result->data = strsep(&msg, "#");
	}

	unsigned int size = atoi(str_size);
	result->size = size;

	printf("node: op code -> %d; addr -> %d, size -> %d\n",  result->code, result->addr, result->size);
	return result;
}

char* executeOperation(operation_t * op){
	if( op->code == 1){
		return writeToMemory(op->addr, op->size, op->data);
	}else if(op->code == 2){
		op->data = readFromMemory(op->addr, op->size);
		return op->data;
	}else{
		return "Error";
	}
	return "Success";
}

void * process(void * ptr){
	printf("node: starting process\n");
	connection_t * conn;
	operation_t * op;

	if (!ptr) {
		pthread_exit(0); 
	}

	conn = (connection_t *)ptr;

	printf("node: will try to read a master op\n");
    char* msg = readClientMessage(conn->sock);
	printf("node: will try to define op\n");
    op = defineOperation(msg);
	if(op == NULL){
		printf("node: op response -> %s\n", "Error - invalid op");
		sendResponseClient(conn->sock, "Error - invalid op");
		/* close socket and clean up */
		close(conn->sock);
		free(conn);
		pthread_exit(0);
	}
    // try to execute operation
	printf("node: execute op\n");
	char * response = executeOperation(op);	
    //send approprieate response
	printf("node: op response -> %s\n", response);
	sendResponseClient(conn->sock, response);
	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);
}

int main(int argc, char ** argv){
	int sock = -1;
	struct sockaddr_in address;
	int port;
	connection_t * connection;
	pthread_t thread;
	initSharedMemory();
	/* check for command line arguments */
	if (argc != 2){
		fprintf(stderr, "usage: %s port\n", argv[0]);
		return -1;
	}

	/* obtain port number */
	if (sscanf(argv[1], "%d", &port) <= 0){
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0){
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = 8081;

    socklen_t server_len = sizeof(address);
	if (bind(sock, (struct sockaddr *)&address, server_len) < 0){
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port at a max of N clients */
	if (listen(sock, N) < 0){
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);
	
	while (1){
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = acceptClientConnection(&address, sock);
		if (connection->sock <= 0){
			free(connection);
		}
		else{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}
	destroySharedMemory();
	return 0;
}