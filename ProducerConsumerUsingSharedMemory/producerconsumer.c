
#include "producerconsumer.h"
/*
 * author       : BhagavathiDhass Thirunavukarasu
 * student ID   : 5078702
 * Csci 5103 Operating System assignment1
 * FunctionName : main
 * Transfer data between producer
 * and consumer process using shared memory and SIGNALS
 * Parameters   : input filename
 */
int main(int argc, char * argv[]) { 

	// Declaring Variables
	pid_t pid;
	FILE *input_file, *output_file;
	char file_buffer[MAX_BUFFER_SIZE];
	int shared_mem_id;
	key_t shared_mem_key = SHM_KEY;
	void *shared_mem_ptr = (void *) 0;
	int flag = 1023;   
	struct shared_mem_struct *shared_mem;
	double time_in_micro_sec;
	double existing_buffer_value;

	//Validations
	if (argc == 1) {
		printf("Error: Please provide input filename\n");
		exit(EXIT_FAILURE);
	}
	if (argc != 2) {
		printf("Error: Incorrect number of arguments passed\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the SIGUSR1 and SIGUSR2 signals
	initSignals(); 
	//Fork a new process
	if ((pid = fork()) < 0) { // if fork fails
		perror("fork");
		exit(EXIT_FAILURE);
	}
	/* Parent Process */
	else if (pid != 0) {
		//Open the input file
		if ((input_file = fopen(argv[1], "r")) == NULL) { // if fopen fails
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		// Creating shared memory segment 
		if ((shared_mem_id = shmget(shared_mem_key, sizeof(struct shared_mem_struct),
		flag)) < 0) { // if shmget fails
			perror("shmget");
			exit(EXIT_FAILURE);
		}
		// Attaching the shared memory to data space 
		if ((shared_mem_ptr = shmat(shared_mem_id, (void *) 0, 0)) == (void *) -1) { // if shmat fails
			perror("shmat");
			exit(EXIT_FAILURE);
		}
		// Typecast the shared memory to our structure
		shared_mem = (struct shared_mem_struct *) shared_mem_ptr;
		// Set the counter to 0
		shared_mem->count = 0; 

		// Loop till EOF
		while (fgets(file_buffer, (MAX_BUFFER_SIZE - 1), input_file) != NULL) {
			//Wait for the consumer to read from the shared memory
			while (shared_mem->count != 0) {
				waitForConsumer();
			}		
			// Copy from buffer to shared memory
			strcpy(shared_mem->buffer, file_buffer);
			// Get the time of the day in microsec
			gettimeofday(&time, NULL);
			time_in_micro_sec = (time.tv_sec) * 1000000 + (time.tv_usec);
			//set the start time in shared memory
			shared_mem->start_time_in_microsec = time_in_micro_sec;
			// Set the count to bytes copied to shared memory 
			shared_mem->count = strlen(shared_mem->buffer);
			//Signal consumer to read from shared memory
			signalConsumer(pid);
		}

		// Wait for the consumer to read
		while (shared_mem->count != 0) {
			waitForConsumer();
		}
		// Set the count to -1 it is EOF 
		shared_mem->count = -1;
		signalConsumer(pid);
		// Wait for comsumer to finish 
		waitpid(pid, NULL, 0);
		// Close the input file
		fclose(input_file);
		printf("Success: File copied to output.txt.\n");
		printf("Buffer time (microsecs) : %f \n", shared_mem->buffer_time);

		// detach the shared memory 
		if (shmdt(shared_mem) == -1) {
			perror("shmdt");
			exit(EXIT_FAILURE);
		}
		
		if (shmctl(shared_mem_id, IPC_RMID, 0) == -1) {
			perror("shmctl");
			exit(EXIT_FAILURE);
		}
		
	} 
	/* Child Process */
	else {  
                //Open ouput file in write mode
		if ((output_file = fopen("output.txt", "w")) == NULL) { //if fopen fails
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		//Creating shared memory segment
		if ((shared_mem_id = shmget(shared_mem_key, sizeof(struct shared_mem_struct),
		flag)) < 0) { // if shmget fails
			perror("shmget");
			exit(EXIT_FAILURE);
		}
		// Attaching the shared memory to data space
		if ((shared_mem_ptr = shmat(shared_mem_id, (void *) 0, 0)) == (void *) -1) { // if shmat fails
			perror("shmat");
			exit(EXIT_FAILURE);
		}
		// Typecast shared memory to shared mem structure 
		shared_mem = (struct shared_mem_struct *) shared_mem_ptr;
		
		//Loop till the EOF is reached
		while (shared_mem->count != -1) {
			// Wait for the producer to write
			while (shared_mem->count == 0) {
				waitForProducer();
			}
			// If not EOF
			if (shared_mem->count != -1) {
				// Copy the data from shared memory buffer to ouput file 
				fputs(shared_mem->buffer, output_file);
				shared_mem->count = 0;

				//Calculate the time for which the data was in shared memory
				gettimeofday(&time, NULL);
				time_in_micro_sec = (time.tv_sec) * 1000000 + (time.tv_usec);

				//set the start time
				printf("Temp cumulative buffer_time : %f \n",shared_mem->buffer_time);
				existing_buffer_value = shared_mem->buffer_time;
				shared_mem->buffer_time = existing_buffer_value + 
                                                          (double)(time_in_micro_sec - shared_mem->start_time_in_microsec);
				//signal producer that consumer has finished consuming
				signalProducer(getppid()); 
			}
		}
		// Close the output file
		fclose(output_file); 
		// detach the shared memory 
		if (shmdt(shared_mem) == -1) {
			perror("shmdt");
			exit(EXIT_FAILURE);
		}
		// Kill the consumer process
		kill(getpid(), SIGTERM); 
	}
	exit(EXIT_SUCCESS); 
}
