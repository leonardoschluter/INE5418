Solução: 

Será um cluster tendo um nó mestre e nós trabalhadores. 
O nó mestre,a princípio, será definido rodando código específico em um servidor. 
mas se tiver tempo, o nó mestre será definido através de protocolo de comunicação entre as diferentes instâncias de servidor.

# Requisitos nó mestre - memory:
 - calculate total memory counting every node
 - define the memory size k of each section(job-node)
 - get a random jobNode to log all of the memory and send request to jobNode. 
 - accept N connections from clients
 - define logical address where the addr of each node starts at -> nodeNumber*K and ends at -> nodeNumber*(K+1) - 1;
 - Then, only deal with logicalAddresses in the master node;
 - Must be able to create N connections with nodes, N = clients connection ( this connections may be with the same node);

# Requisitos nó trabalhador - memory: 
 - write and read to/from the node memory based on size, realAddress and data
 - Capability of receveing N reads at the same time, where N = number of clients.
 - Only one writing at a time;
 - lock a memory region when writing to it.
 - reads must wait when a region is locked. ( use sempahore )
 - in each node, its real memory starts from 0 to K - 1

# Message Pattern between master and node
 #op#addr#size#data
 1. op -> 1 - write or 2 - read 
 2. addr -> logicalAddr in the master
 3. size -> unsigned int
 4. data -> char* optional ( will be used only on write )
A aplicação de log será feito usando a arquitetura peer-to-peer


# TODO
[ ] - aggregate responses from read of multiple nodes
[ ] - make the nodes and the master more tolerant when detecting an error
 