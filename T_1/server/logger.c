

//TODO loggerMaster
// 1. create array with all nodes 
// 2. create a thread for each node besides this one
// 3. asks for full memory read of each loggerNode
// 4. join Threads
// 5. concat result
// 6. output for a file

//TODO loggerNode
// 1. change each node to start a new process - logger
// 2. create a shared memory with the node process
// 3. when receiving a read from masterLogger, simply use readMemory from memory.h

#include <stdio.h> 
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include "../util/string_fn.h"
#include "../util/client_socket.h"

int N_NODES = 2;
int NODE_SIZE = 32;

typedef struct NodeS {
	char* addr;
	short unsigned int port;
} node_t;

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

int main(){
    struct NodeS* nodes = malloc(sizeof(struct NodeS)*N_NODES);
	struct NodeS* node = malloc(sizeof(struct NodeS)); 
	node->addr = "192.168.0.53";
	node->port = 8081;
	nodes[0] = *node;


	node->addr = "192.168.0.54";
	node->port = 8081;
	nodes[1] = *node;
}