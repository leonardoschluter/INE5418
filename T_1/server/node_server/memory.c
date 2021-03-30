
#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h>
#include <string.h> 
#include <sys/sem.h>
#include "memory.h"

int K = 32;
sem_t WRITER;
sem_t READER;
int RC;
char* memory;

char* readFromMemory(int addr, unsigned int size){
    int realAddr = calculateRealAddress(addr);
    if( size > K - realAddr){ // Trying to read more than it can ... 
        return "Error"; 
    }
    char* response = malloc(size);

    sem_wait(&READER);
    RC = RC + 1;
    if(RC == 1){
        sem_wait(&WRITER);
    }
    sem_post(&READER); 

    for(int i = 0; i < size ; i++ ){
        int memIndex = realAddr + i;
        response[i] = memory[memIndex];
    }
    
    sem_wait(&READER);
    RC = RC - 1;
    if(RC == 0){
        sem_post(&WRITER);
    }
    sem_post(&READER);
	return response;
}

char* writeToMemory(int addr, unsigned int size, char* data){
	printf("node: starting write to memory \n");
    int realAddr = calculateRealAddress(addr);
	printf("node: realAdd -> %d\n", realAddr);
    if(realAddr < 0 || realAddr >= K ){
		printf("node: Out of bounds\n");
        return "Error"; 
    }
    size_t realSize = calculateSizeOfData(data);
	printf("node: realSize -> %ld\n", realSize);
    if(realSize != (size_t)size || realSize > (size_t) K - realAddr){
		printf("node: You shall not pass\n");
        return "Error";
    }

	printf("node: entering critical section\n");
    sem_wait(&WRITER);
    // START Critical section
    for(int i = realAddr; i < realAddr + realSize; i ++){
		printf("node: write in position %d the char %c\n", i, data[i-realAddr]);
        memory[i] = data[i-realAddr];
    }
    // END critical section

	printf("node: leaving critical section\n");
    sem_post(&WRITER);
	return "Success";
}

void initSharedMemory(){
    sem_init(&WRITER, 0, 1);
    sem_init(&READER, 0, 1);
    RC = 0;
    memory = malloc(K);
}

void destroySharedMemory(){
    sem_destroy(&WRITER);
    sem_destroy(&WRITER);
}

unsigned int calculateRealAddress(int addr){
    return addr % K;
}

size_t calculateSizeOfData(char* data){
    return strnlen(data, K + 1);
}
