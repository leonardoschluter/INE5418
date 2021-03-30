

char* writeToMemory(int addr, unsigned int size, char* data);

char* readFromMemory(int addr, unsigned int size);

void destroySharedMemory();

void initSharedMemory();

unsigned int calculateRealAddress(int addr);

size_t calculateSizeOfData(char* data);