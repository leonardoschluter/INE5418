all: 
	gcc -Wall -o client/application client/application.c client/util/client.c
	gcc -Wall -o server/node server/node_server/node.c server/util/server_socket.c server/node_server/memory.c -lpthread -lrt
	gcc -Wall -o server/master server/master_server/master.c server/util/server_socket.c server/util/client_socket.c server/util/string_fn.c server/master_server/controller.c -lpthread -lrt
master:
	gcc -Wall -o server/master server/master_server/master.c server/util/server_socket.c server/util/client_socket.c server/util/string_fn.c server/master_server/controller.c -lpthread -lrt
clean: 
	rm server/node 
	rm server/master
	rm client/application