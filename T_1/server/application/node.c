#include <stdio.h> 
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 

int K = 32;
sem_t WRITER;
sem_t READER;
int RC;
char memory[K];

void initSharedMemory(){
    sem_init(&WRITER, 0, 1);
    sem_init(&READER, 0, 1);
    RC = 0;
    memory = malloc(K);
}

void destroySharedMemory(){
    delete(memory);
    sem_destroy(&WRITER);
    sem_destroy(&WRITER);
}

int calculateRealAddress(int addr){
    return addr % K;
}

size_t calculateSizeOfData(char* data){
    return strnlen(data, K + 1);
}

char* readFromMemory(int addr, unsigned int size){
    int realAddr = calculateRealAddress(addr);
    if( size >= K - realAddr){ // Trying to read more than it can ... 
        return; // TODO add easter egg --> "YOU SHALLLLL NOT PASSSSSS !!!!!!"
    }
    char* response = malloc(size);

    sem_wait(READER);
    RC = RC + 1;
    if(RC == 1){
        sem_wait(WRITER)
    }
    sem_post(READER); 

    for(int i = 0; i < size - realAddr ; i++ ){
        int memIndex = realAddr + i;
        response[i] = memory[memIndex];
    }
    
    sem_wait(READER);
    RC = RC - 1;
    if(RC == 0){
        sem_post(WRITER);
    }
    sem_post(READER);
}

char* writeToMemory(int addr, unsigned int size, char* data){
    // TODO checkups on size/data/addr
    int realAddr = calculateRealAddress(addr);
    if(realAddr < 0 || realAddr >= K ){
        return; // TODO return error message of out bounds to master;
    }
    size_t realSize = calculateSizeOfData(data);
    if(realSize != (size_t)size || realSize > (size_t) K - realAddr){
        return; // TODO add easter egg --> "YOU SHALLLLL NOT PASSSSSS !!!!!!"
    }

    sem_wait(WRITER);
    // START Critical section
    for(i = realAddr; i < realSize; i ++){
        memory[i] = data;
    }
    // END critical section
    sem_post(WRITER);
}