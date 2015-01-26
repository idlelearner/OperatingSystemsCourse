#include "sharedStructures.h"

pthread_mutex_t lock;
pthread_cond_t spaceAvailable, itemAvailable;

int main(int argc, char* argv[]) {

	int shmem_id; /* shared memory identifier */
	void *shmem_ptr; /* pointer to shared segment */

	key_t key; /* A key to access shared memory segments */
	int size; /* Memory size needed, in bytes */
	int flag; /* Controls things like r/w permissions */
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;
	pid_t pid1,pid2,pid3,pid4;
	int status;
	key = 44125; /* Some arbitrary integer, which will also be
	 passed to the other processes which need to
	 share memory */
	size = 1024; /* Assume we need 4Kb of memory, which means we
	 can store 512 integers or floats */
	flag = 1023; /* 1023 = 111111111 in binary, i.e. all permissions
	 and modes are set. This may not be appropriate
	 for all programs! */

	/* First, create a shared memory segment */
	shmem_id = shmget(key, size, flag);
	if (shmem_id == -1) {
		perror("shmget failed");
		exit(1);
	}

	//printf ("Got shmem id = %d\n", shmem_id);

	/* Now attach the new segment into my address space.
	 This will give me a (void *) pointer to the shared memory area.
	 The NULL pointer indicates that we don't care where in the address
	 space the new segment is attached. The return value gives us that
	 location anyway.
	 */

	shmem_ptr = shmat(shmem_id, (void *) NULL, 1023);
	if (shmem_ptr == (void *) -1) {
		perror("shmat failed");
		exit(2);
	}

	//printf ("Got ptr = %p\n", shmem_ptr);

	pthread_mutexattr_init(&mutex_attr);
	pthread_condattr_init(&cond_attr);
	
	//make the scope of the mutex and conditonal variable as process visible
	//so that the threads from different process can see the change
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);

	pthread_mutex_init(&lock, &mutex_attr);
	pthread_cond_init(&spaceAvailable, &cond_attr);
	pthread_cond_init(&itemAvailable, &cond_attr);

	struct sharedStructure shStr = {2, 0, 0, 0, lock,
			spaceAvailable, itemAvailable };

	*((struct sharedStructure*) shmem_ptr) = shStr;

	char keystr[10];
	sprintf(keystr, "%d", key);

	if((pid1 = fork()) == -1)
   	{ 
	      perror("Error while forking producer_RED");
	      exit(1);
     	}
	else if (pid1 == 0)
	{
	   if(execl("./producer", "producer", keystr, "RED", (char*) 0) == -1)
	   {
	       perror("execl failed for producer RED");      
	       exit(2);
            }
	}
	if((pid2 = fork()) == -1)
   	{ 
	      perror("Error while forking producer_WHITE");
	      exit(1);
     	}
	else if (pid2 == 0)
	{
	   if(execl("./producer", "producer", keystr, "WHITE", (char*) 0) == -1)
	   {
	       perror("execl failed for producer WHITE");       
	       exit(2);
	    }
	}
	if((pid3 = fork()) == -1)
   	{ 
	      perror("fork--producer_RED");
	      exit(1);
     	}
	else if (pid3 == 0)
	{
	    if(execl("./producer", "producer", keystr, "BLACK", (char*) 0) == -1)
	    {
	       perror("execl failed for producer BLACK");		       
	       exit(2);
	     }
	}
	if((pid4 = fork()) == -1)
        {
             perror("fork--consumer");
             exit(1);
        }
   	else if (pid4 == 0)
   	{
      		if(execl("./consumer", "consumer", keystr, (char*) 0) == -1)
                {
                	perror("execl failed for consumer");     
	                exit(2);
               }
        }
	pid2 = waitpid(-1, &status, 0); /* Wait for termination of producer_WHITE process */
        pid3 = waitpid(-1, &status, 0); /* Wait for termination of producer_BLACK process */
        pid4 = waitpid(-1, &status, 0);  /* Wait for termination of consumer process */
	shmdt((void *) shmem_ptr);
		
	return 0;
}
