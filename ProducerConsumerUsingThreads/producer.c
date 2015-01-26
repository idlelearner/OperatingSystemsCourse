
#include "sharedStructures.h"

#ifndef IPC_ALLOC
#define IPC_ALLOC 0100000
#endif

#define TIME(a,b) ((a*1000000ull) + b)
#define C(X) ((struct sharedStructure *)X)

/* This is made global so that it can be used across all functions easily */
void *smp; /* pointer to shared memory */

void* producer(void *arg) {
	int i;
	int c;
	char *color = (char *)arg;
	printf("\nProducer : %s started",color);
	char filename[40];
	sprintf(filename, "Producer_%s.txt", color);

	FILE *producerFile = fopen(filename, "w");
	if (producerFile == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}

	for (i = 0; i < 1000; i++) {
		//printf(" Producer : %s : %d",color,i);
		//Acquire lock before writing
		pthread_mutex_lock(&C(smp)->lock);
		//Wait if there is no space in buffer
		while ( C(smp)->count == C(smp)->N )
			while (pthread_cond_wait(&C(smp)->spaceAvailable, &C(smp)->lock)!= 0);
		struct timeval timeStamp;
		gettimeofday(&timeStamp, NULL);
		c = C(smp)->in;
		strcpy(C(smp)->buffer[c].str, color);
		C(smp)->buffer[c].timeStamp = TIME(timeStamp.tv_sec, timeStamp.tv_usec);
		//printf(" %s %llu\n", C(smp)->buffer[c].str,C(smp)->buffer[c].timeStamp);
		fprintf(producerFile, "%s %llu\n", C(smp)->buffer[c].str,C(smp)->buffer[c].timeStamp);
		C(smp)->count++;
		C(smp)->in = (C(smp)->in+1)%C(smp)->N;
		//release the lock after writing
		pthread_mutex_unlock(&C(smp)->lock);
		//Signal the consumer that items are available
		pthread_cond_signal(&C(smp)->itemAvailable);
	}

	fclose(producerFile);
	printf("\nFinished execution! Output written to Producer_%s.txt\n",
			(char*) arg);

	return 0;
}



int main(int argc, char *argv[]) {
	//printf("producer process\n");
	int id; /* shared memory identifier */

	/* A key for the shared memory segment has been passed to this program
	 as its first parameter. We use it to get the segment id of the
	 segment that the parent process created. The size parameter is set
	 to zero, and the flag to IPC_ALLOC, indicating that the segment is
	 not being created here, it already exists
	 */

	id = shmget(atoi(argv[1]), 0, IPC_ALLOC);
	if (id == -1) {
		perror("child shmget failed");
		exit(1);
	}

	//printf ("child Got shmem id = %d\n", id);

	/* Now attach this segment into the address space. Again, the 1023 is a
	 flag consisting of all 1s, and the NULL pointer means we don't care
	 where in the address space the segment is attached
	 */
	smp = shmat(id, (void *) NULL, 1023);
	if (smp == (void *) -1) {
		perror("child shmat failed");
		exit(2);
	}

	//printf ("prod child Got ptr = %p\n", smp);
	pthread_t prod; /* producer thread variables */
	pthread_attr_t attr; /*attribute object*/
	pthread_attr_init(&attr);

	int n;

	/*  Create blue/red producer thread                        */
	if ((n = pthread_create(&prod, &attr, producer, (void*) argv[2]))
			!= 0) {
		fprintf(stderr, "pthread_create :%s\n", strerror(n));
		return 1;
	}

	/* wait for the thread to complete */
	if ((n = pthread_join(prod, NULL)) != 0) {
		fprintf(stderr, "pthread_join:%s\n", strerror(n));
		exit(1);
	}
	
	sleep(1);
	shmdt((void *) smp);

	return 0;
}
