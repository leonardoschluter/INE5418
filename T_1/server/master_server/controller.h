typedef struct NodeS {
	char* addr;
	short unsigned int port;
} node_t;

typedef struct ConnectionClient{
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


operation_t * defineOperation(char* msg);

void * executeOperation(void * op_ptr);

operation_t * breakOperation(operation_t * op, int node);

operation_t * convertSingleToMultiOperations(int numberOfNodes, operation_t * op);

char * dispatchOperations(int numberOfNodes, operation_t * nodeOps, node_t * nodes);

char * dispatchSingleOperation(node_t * nodes, operation_t * op);
