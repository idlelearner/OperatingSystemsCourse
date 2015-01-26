#include "sharedStructures.h"

#ifndef IPC_ALLOC
#define IPC_ALLOC 0100000
#endif

#define TIME(a,b) ((a*1000000ull) + b)
#define C(X) ((struct sharedStructure *)X)

/* This is made global so that it can be used across all functions easily */
void *smp; /* pointer to shared memory */

void* consumer(void *arg) {
	int i;
	int c;
	FILE *consumerFile = fopen("Consumer.txt", "w");
	printf("\nConsumer started :");
	if (consumerFile == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	for (i = 1; i <= 3000; i++) {
		//printf("\nConsumer : %d", i);
		//Acquire lock before reading
		pthread_mutex_lock(&C(smp)->lock);	
		// Wait if there are no items available.
		while(C(smp)->count==0)
			while (pthread_cond_wait( &C(smp)->itemAvailable, &C(smp)->lock)!= 0);
		c = C(smp)->out;
		//printf("\nConsumer %s %llu\n", C(smp)->buffer[0].str,C(smp)->buffer[0].timeStamp);
		fprintf(consumerFile, "%s %llu\n", C(smp)->buffer[0].str, C(smp)->buffer[0].timeStamp);
		C(smp)->out = (C(smp)->out+1)%C(smp)->N;
		C(smp)->count--;
		//release the lock
		pthread_mutex_unlock(&C(smp)->lock);
		//signal that the space is available.
		pthread_cond_signal(&C(smp)->spaceAvailable);
	}


	fclose(consumerFile);

	printf("Finished execution! Output written to Consumer.txt\n");

	return 0;
}

int main(int argc, char *argv[]) {

	int id; /* shared memory identifier */
	int n;
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
#ifdef DEBUG
	printf ("child Got shmem id = %d\n", id);
#endif

	/* Now attach this segment into the address space. Again, the 1023 is a
	 flag consisting of all 1s, and the NULL pointer means we don't care
	 where in the address space the segment is attached
	 */
	smp = shmat(id, (void *) NULL, 1023);
	if (smp == (void *) -1) {
		perror("child shmat failed");
		exit(2);
	}

	//printf ("child Got ptr = %p\n", smp);

    pthread_t cons; /* thread variables */
	pthread_attr_t attr; /*attribute object*/
	pthread_attr_init(&attr);

	printf("\nCreating consumer process\n");
	/*  Create consumer thread                        */
	if ((n = pthread_create(&cons, &attr, consumer, NULL)) != 0) {
		fprintf(stderr, "pthread_create :%s\n", strerror(n));
		return 1;
	}

	/* wait for the thread to complete */
	if ((n = pthread_join(cons, NULL)) != 0) {
		fprintf(stderr, "pthread_join:%s\n", strerror(n));
		exit(1);
	}

	/* this is just to make sure that all producer processes have exited */
	sleep(1);

	shmdt((void *) smp);

	/* By now all processes must have exited, so cleanup the shared memory */
	shmctl(id, IPC_RMID, NULL);
	exit(EXIT_SUCCESS);
}


