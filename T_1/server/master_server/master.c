#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/sem.h>
#include <linux/in.h>
#include <string.h>
#include "../util/server_socket.h"
#include "../util/client_socket.h"
#include "../util/string_fn.h"

int N_CLIENTS = 10;
int N_NODES = 2;
int NODE_SIZE = 32;

typedef struct NodeS {
	char* addr;
	short unsigned int port;
} node_t;

typedef struct{
	int sock;
	struct sockaddr address;
	int addr_len;
	node_t* nodes;
} connection_client;

typedef struct NodeOperation{
    int code;
	int node_id;
	node_t * node;
	int addr;
    unsigned int size;
    char* data;
	char* msg;
	char* response;
} operation_t;

operation_t * defineOperation(char* msg){	
	operation_t * result = (operation_t *)malloc(sizeof(operation_t));
	result->msg = strdup(msg);
    printf("defineOperation da msg -> -%c-\n", msg[0]);
    char * code = strsep(&msg, "#");
	printf("server: operation code -%s- \n", code);
	if( *code != '1' && *code != '2'){
		// TODO send response of bad request

		printf("server: invalid operation code -%s- \n", code);
		pthread_exit(0);
	}
	result->code = atoi(code);

	printf("server: calculating addr \n");
	result->addr = atoi(strsep(&msg, "#"));
	printf("server: operation addr %i \n", result->addr);
    char * str_size = strsep(&msg, "#");
	printf("server: operation size %s \n", str_size);

	result->node_id = result->addr / NODE_SIZE;
	if(*code == '1'){
		result->data = strsep(&msg, "#");
		printf("server: data -> %s\n", result->data);
	}
	result->size = atoi(str_size);
	return result;
}

char * buildNodeCommand(operation_t * op){
    char * buffer = malloc( sizeof(char*)*(128));
    bzero(buffer, sizeof(char*)*(128));

    snprintf(buffer, sizeof(char*)*(128), "%d#%d#%d#%s", op->code, op->addr, op->size, op->data);
	printf("server: buildNodeCommand result = %s\n", buffer);
	return buffer;
}


void * executeOperation(void * op_ptr){
	operation_t * op = (operation_t *) op_ptr;
	node_t * node = op->node;
	connection_node * nodeConn = connectToServer(node->addr, node->port);
	printf("server: node connection sock %d\n", nodeConn->sock);

	//buildNodeMsg
	char * nodeMsg = buildNodeCommand(op);

	// send msg to node
	sendMessage(nodeMsg, nodeConn->sock);

	// read response ( write - if was sucessfull, read - msg)
	char * response = readMessage(nodeConn->sock);
	op->response = response;
	closeConnection(nodeConn->sock);
	free(nodeConn);
	pthread_exit(0);
}

operation_t * breakOperation(operation_t * op, int node){
	operation_t * result = (operation_t *)malloc(sizeof(operation_t));
	int currentOpSize = 0;
	if(op->size + ( op->addr % NODE_SIZE ) > NODE_SIZE){
		currentOpSize = NODE_SIZE - ( op->addr % NODE_SIZE );
	}else{
		currentOpSize = op->size;
	}
	int nextOpAddr = NODE_SIZE * node;
	int nextOpNodeId = node + 1;
	int nextOpSize = op->size - currentOpSize;
	
	result->size = currentOpSize;
	result->addr = op->addr;
	result->node_id = node;
	result->code = op->code;
	result->msg = op->msg;

	op->size = nextOpSize;
	op->addr = nextOpAddr; 
	op->node_id = nextOpNodeId;


	printf("server: breakOp result code -> %d; node -> %d, addr -> %d, size -> %d\n", result->code, result->node_id, result->addr, result->size);
	if(op->code == 1){
		result->data = substring(op->data, 1, currentOpSize);
		op->data = substring(op->data, currentOpSize+1, strlen(op->data)-currentOpSize);
		printf("server: breakOp result data -> %s;\n\n",  result->data);	
	}
	
	return result;	
}

char * defineResult(char * responses){
	if(strstr(responses, "error") != NULL || strstr(responses, "PASS") != NULL ){
		return "Error - You shall not pass !";
	}else if(strstr(responses, "success") != NULL){
		return "Sucess";
	}else {
		return responses;
	}
}

void * process(void * ptr){
	int len;
	connection_client * conn;
	operation_t * op;
	operation_t * nodeOps;
	long addr = 0;
	int numberOfNodes = 0;

	if (!ptr) {
		pthread_exit(0); 
	}

	conn = (connection_client *)ptr;

    char *buffer = readClientMessage(conn->sock);

    op = defineOperation(buffer);
	if(op->size + (op->addr % NODE_SIZE) > NODE_SIZE - 1){
		numberOfNodes = 1 + (( op->size - 1 + (op->addr % NODE_SIZE) ) / NODE_SIZE ) ;
		nodeOps = malloc(sizeof(operation_t)*numberOfNodes);
		int i = 0;
		for(i = 0; op->size + (op->addr % NODE_SIZE) > NODE_SIZE ; i++){
			nodeOps[i] = *breakOperation(op, op->node_id);
		}

		if(op->size>0){
			nodeOps[i] = *breakOperation(op, op->node_id);
		}

	}

    char * responses = malloc( sizeof(char*)*(128));
	if(numberOfNodes>0){
		printf("server: number of nodes involed in  op is %d", numberOfNodes);
		// TODO create thread

		pthread_t* tid = malloc(sizeof(pthread_t)*numberOfNodes);
		for(int i = 0; i < numberOfNodes; i++){
			operation_t * currentOp = &nodeOps[i];
			currentOp->node = &conn->nodes[currentOp->node_id];
			pthread_create((pthread_t *)&tid[i], 0, executeOperation, currentOp);
		}
		// join threads and concat currentOp->responses
		for(int i = 0; i < numberOfNodes; i++){
			operation_t * currentOp = &nodeOps[i];
   			pthread_join(tid[i], NULL);
			if(currentOp->code == 2){
				strncat(responses, currentOp->response, currentOp->size);
			}else{
				strncat(responses, currentOp->response, 7);
			}
		}

	}else{
		op->node = &conn->nodes[op->node_id];

		pthread_t* tid = malloc(sizeof(pthread_t));
		pthread_create(tid, 0, executeOperation, op);
		pthread_join(tid, NULL);
		executeOperation(op);
		responses = op->response;
	}

	char * result = defineResult(responses);
	sendResponseClient(conn->sock, result);

	printf("server: op response %s\n", responses);
	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);


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