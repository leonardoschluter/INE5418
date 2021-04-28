# To execute
1. run `make all` in root folder
2. share this same folder between nodes in VM.
3. start as many nodes as you want, keep their IP
3.1 to start just use `./server/node'
4. update master.c with the nodes you added as the examples I've left there and update the constant N_NODES with the new number of nodes
5. run `make master` 
6. run `./server/master 8080`
7. run `./client/application`
8. enjoy :) 

# Problems with this solution
1. It has hardcoded nodes and quantity of nodes.
2. It has a master server, meaning that this will be the solution bottleneck.
3. The size of each node memory is hardcoded in master.c and node.c
4. The connection that the application tries to create is not configurable, meaning that it will always connect to the same place.
5. There is some space for buffer overflow attack in this solution ( I've tried to defend, but there is some flaws yet) - be my guest to find them ;) 
6. We could easily DDoS this solution, so there is another flaw in security. Solution would be to limitate the number of clients trying to execute.
7. As I'm doing all of this by myself I did not have the time to create an logger, so this is not tolerant to faults and we cannot audit this memory.

Solução: 

Será um cluster tendo um nó mestre e nós trabalhadores. 
O nó mestre,a princípio, será definido rodando código específico em um servidor. 
mas se tiver tempo, o nó mestre será definido através de protocolo de comunicação entre as diferentes instâncias de servidor.

# Requisitos nó mestre - memory:
 - calculate total memory counting every node
 - define the memory size k of each section(job-node)
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
 op#addr#size#data
 1. op -> 1 - write or 2 - read 
 2. addr -> logicalAddr in the master
 3. size -> unsigned int
 4. data -> char* optional ( will be used only on write )
 
<!--
TODO
1. finish logger
2. Report limitations of arch
3. clean mallocs
-->
