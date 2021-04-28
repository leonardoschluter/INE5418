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

int NODE_SIZE = 32;

char * buildNodeCommand(operation_t * op){
    char * buffer = malloc( sizeof(char*)*(128));
    bzero(buffer, sizeof(char*)*(128));

    snprintf(buffer, sizeof(char*)*(128), "%d#%d#%d#%s", op->code, op->addr, op->size, op->data);
	printf("server: buildNodeCommand result = %s\n", buffer);
	return buffer;
}

operation_t * defineOperation(char* msg){	
	operation_t * result = (operation_t *)malloc(sizeof(operation_t));
	result->msg = strdup(msg);
    printf("defineOperation da msg -> -%c-\n", msg[0]);
    char * code = strsep(&msg, "#");
	printf("server: operation code -%s- \n", code);
	if( *code != '1' && *code != '2'){
		printf("server: invalid operation code -%s- \n", code);
		return NULL;
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

operation_t * convertSingleToMultiOperations(int numberOfNodes, operation_t * op){
	operation_t * nodeOps = malloc(sizeof(operation_t)*numberOfNodes);
	int i = 0;
	for(i = 0; op->size + (op->addr % NODE_SIZE) > NODE_SIZE ; i++){
		nodeOps[i] = *breakOperation(op, op->node_id);
	}

	if(op->size>0){
		nodeOps[i] = *breakOperation(op, op->node_id);
	}
	return nodeOps;
}

char * dispatchOperations(int numberOfNodes, operation_t * nodeOps, node_t * nodes){
    char * responses = malloc( sizeof(char*)*(128));
	printf("server: number of nodes involed in  op is %d", numberOfNodes);
		pthread_t* tid = malloc(sizeof(pthread_t)*numberOfNodes);
		for(int i = 0; i < numberOfNodes; i++){
			operation_t * currentOp = &nodeOps[i];
			currentOp->node = &nodes[currentOp->node_id];
			pthread_create((pthread_t *)&tid[i], 0, executeOperation, currentOp);
		}
		for(int i = 0; i < numberOfNodes; i++){
			operation_t * currentOp = &nodeOps[i];
   			pthread_join(tid[i], NULL);
			if(currentOp->code == 2){
				strncat(responses, currentOp->response, currentOp->size);
			}else{
				strncat(responses, currentOp->response, 7);
			}
		}
	return responses;
}
char * dispatchSingleOperation(node_t * nodes, operation_t * op){
	op->node = &nodes[op->node_id];
	pthread_t* tid = malloc(sizeof(pthread_t));
	pthread_create(tid, 0, executeOperation, op);
	pthread_join(*tid, NULL);
	return op->response;
}