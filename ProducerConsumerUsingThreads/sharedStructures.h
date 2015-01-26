
#ifndef SHAREDSTRUCTURES_H_
#define SHAREDSTRUCTURES_H_

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

//Shared buffer
struct bufferItem {
	char str[10];
	unsigned long long timeStamp;
};

//Structure for shared memory
struct sharedStructure {
	int N;
	int in; 
	int out;
	int count;
	pthread_mutex_t lock;
	pthread_cond_t spaceAvailable, itemAvailable;
	struct bufferItem buffer[2];
};

#endif /* SHAREDSTRUCTURES_H_ */
