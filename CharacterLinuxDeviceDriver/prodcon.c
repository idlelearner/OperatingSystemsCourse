/* Producer-Consumer problem using POSIX Threads */

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <fcntl.h>

const int N = 1000;
char fileName[] = "/dev/scullBuffer0";

void* producer(void *arg) {
	int i = 0, processNo = atoi((char*) arg);

	free((char*) arg);

	char filename[40];
	sprintf(filename, "Producer_%d.txt", processNo);

	FILE *producerFile = fopen(filename, "w");
	if (producerFile == NULL) {
		printf("Producer #%d encountered an error while opening %s!\n",
				processNo, filename);
		exit(1);
	}

	char buf[512] = { '0' };

	// Open scullBuffer in write mode
	int fd = open(fileName, O_WRONLY);

	if (fd == 0) {
		//Couldn't open scullBuffer
		fprintf(producerFile,
				"Producer #%d encountered an error while opening scullBuffer\n",
				processNo);
	}

	// Continuously write up to WRITE times. This should be run parallel with consumer
	while (i < N) {
		int writeval = write(fd, buf, 512);

		if (writeval == 0) {
			fprintf(producerFile,
					"Producer buffer is full and there are no Consumer processes\n");
		} else if (writeval < 0) {
			fprintf(stderr,
					"Producer #%d encountered an error while writing to scullBuffer\n",
					processNo);
		} else {
			fprintf(producerFile,
					"Entry #%d: Producer #%d finished writing %d bytes to scullBuffer\n",
					i+1, processNo, writeval);
			++i;
		}
	}

	// Close when finished
	close(fd);

	printf(
			"Producer #%d wrote %d items to the scullBuffer, the item logs are available in %s\n",
			processNo, i, filename);
	fclose(producerFile);

	return 0;
}

void* consumer(void* arg) {
	int i = 0, processNo = atoi((char*) arg);
	free((char*) arg);

	char buf[512];

	char filename[40];
	sprintf(filename, "Consumer_%d.txt", processNo);

	int starvationCount = 0;

	FILE *consumerFile = fopen(filename, "w");
	if (consumerFile == NULL) {
		printf("Consumer #%d encountered an error while opening %s!\n",
				processNo, filename);
		exit(1);
	}

	// Open scullBuffer in write mode
	int fd = open(fileName, O_RDONLY);

	if (fd == 0) {
		//Couldn't open scullBuffer
		fprintf(consumerFile,
				"Consumer #%d encountered an error while opening scullBuffer\n",
				processNo);
	}

	// Continuously read until there is nothing in the buffer
	while (1) {
		int readVal = read(fd, buf, 512);

		if (readVal == 0) {
			fprintf(consumerFile,
					"Consumer buffer is empty and there are no Producer processes\n");
			if (++starvationCount == 2000) {
				break;
			}

		} else if (readVal < 0) {
			fprintf(stderr,
					"Consumer #%d encountered an error while reading from scullBuffer\n",
					processNo);
			break;
		} else {
			fprintf(consumerFile,
					"Entry #%d: Consumer #%d finished reading %d bytes from the scullBuffer\n",
					i+1, processNo, readVal);
			++i;
			starvationCount = 0;
		}
	}

	// Close when finished
	close(fd);

	printf(
			"Consumer #%d read %d items from the scullBuffer, the item logs are available in %s\n",
			processNo, i, filename);
	fclose(consumerFile);

	return 0;
}

int main(int argc, char* argv[]) {
	pthread_t prod[3], cons[2]; /* thread variables */
	pthread_attr_t attr; /*attribute object*/
	int n, i;

	pthread_attr_init(&attr);

	/*  Create producer threads                        */
	for (i = 1; i <= 3; ++i) {
		char *buf = (char*) malloc(sizeof(char) * 5);
		sprintf(buf, "%d", i);
		if ((n = pthread_create(&prod[i - 1], NULL, producer, (void*) buf))
				!= 0) {
			fprintf(stderr, "pthread_create :%s\n", strerror(n));
			return 1;
		}
	}

	/*  Create consumer threads                        */
	for (i = 1; i <= 2; ++i) {
		char *buf = (char*) malloc(sizeof(char) * 5);
		sprintf(buf, "%d", i);
		if ((n = pthread_create(&cons[i - 1], NULL, consumer, (void*) buf))
				!= 0) {
			fprintf(stderr, "pthread_create :%s\n", strerror(n));
			return 1;
		}
	}

	/* Wait for the consumer threads to finish.         */
	for (i = 1; i <= 2; ++i) {
		if ((n = pthread_join(cons[i - 1], NULL)) != 0) {
			fprintf(stderr, "pthread_join:%s\n", strerror(n));
			exit(1);
		}
	}

	printf("scullerBuffer Tested successfully! \n");
	return 0;
}
