#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include "../util/server_socket.h"
#include "../util/client_socket.h"
#include "../util/string_fn.h"
#include "controller.h"


int N_CLIENTS = 10;
int N_NODES = 2;
int NODE_SIZE_M = 32;

/*
*
*
* ###### CONTROLLER OF OPERATIONS
*
*
*/

void * process(void * ptr){
	connection_client * conn;
	operation_t * op;
	operation_t * nodeOps;
	int numberOfNodes = 0;

	if (!ptr) {
		pthread_exit(0); 
	}

	conn = (connection_client *)ptr;

    char *buffer = readClientMessage(conn->sock);

    op = defineOperation(buffer);
	if(op == NULL){
		sendResponseAndClose(conn->sock, "Error - Invalid op");
		free(conn);
		pthread_exit(0);
	}

	if(op->size + (op->addr % NODE_SIZE_M) > NODE_SIZE_M - 1){
		numberOfNodes = 1 + (( op->size - 1 + (op->addr % NODE_SIZE_M) ) / NODE_SIZE_M ) ;
		nodeOps = convertSingleToMultiOperations(numberOfNodes, op);
	}

    char * responses = malloc( sizeof(char*)*(128));
	if(numberOfNodes>0){
		dispatchOperations(numberOfNodes, nodeOps, conn->nodes);
	}else{
		dispatchSingleOperation(conn->nodes, op);
	}

	char * result = defineResult(responses);	
	sendResponseAndClose(conn->sock, result);
	free(conn);
	pthread_exit(0);
	return 0;
}

int clientReceiver(int argc, char ** argv, node_t nodes[N_NODES]){
	int sock = -1;
	struct sockaddr_in address;
	int port;

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
    sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock <= 0){
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = 8080;
    
    socklen_t server_len = sizeof(address);
	if (bind(sock, (struct sockaddr *)&address, server_len) < 0){
		fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
		return -4;
	}

	/* listen on port at a max of N clients */
	if (listen(sock, N_CLIENTS) < 0){
		fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
		return -5;
	}

	printf("%s: ready and listening\n", argv[0]);
	
	while (1){
		connection_client * connection;
		pthread_t thread;
		/* accept incoming connections */
		connection = (connection_client *)malloc(sizeof(connection_client));
		
		connection->sock = acceptClientConnection(&address, sock);
		if (connection->sock <= 0){
			free(connection);
		}
		else{
			/* start a new thread but do not wait for it */
			connection->nodes = nodes;
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}	
	return 0;
}

// The master node could be running in the same computer of a node.
// Actually, master could be a process started in only one of the nodes through agreements policies
// such as selecting the node with highiest IP  as the master
int main(int argc, char ** argv){
	// TODO initiate nodes array in an automatic way
	// idea -> broadcast a message, a node may respond with its info ( ip & port )
	struct NodeS* nodes = malloc(sizeof(struct NodeS)*N_NODES);
	struct NodeS* node = malloc(sizeof(struct NodeS)); 
	node->addr = "192.168.0.53";
	node->port = 8081;
	nodes[0] = *node;


	node->addr = "192.168.0.54";
	node->port = 8081;
	nodes[1] = *node;

	clientReceiver(argc, argv, nodes);
	return 1;
}