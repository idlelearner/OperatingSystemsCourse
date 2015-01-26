
#include "producerconsumer.h"
/*
 * author       : BhagavathiDhass Thirunavukarasu
 * student ID   : 5078702
 * Csci 5103 Operating System assignment1
 */

/*
 * FunctionName: signalHandler
 * Handler for SIGUSR1 and SIGUSR2 signals.
 * Parameters: signo - Signalnumber
 * Return type: void
 */
static void signalHandler(int signo) {
        // Set the signal flag
	sigflag = 1; 
}

/*
 * FunctionName: initSignals
 * Handler for SIGUSR1 and SIGUSR2 signals.
 * Parameters: signo - Signalnumber
 * Return type: void
 */
void initSignals() {
	/* Link handler functions */
	if (signal(SIGUSR1, signalHandler) == SIG_ERR) {
		perror("signal");
		exit(EXIT_FAILURE);
	}
	if (signal(SIGUSR2, signalHandler) == SIG_ERR) {
		perror("signal");
		exit(EXIT_FAILURE);
	}

	sigemptyset(&emptymask);
}

/*
 * FunctionName: signalProducer
 * Sends SIGUSR2 signal to producer or parent.
 * Parameters: process id to which the signal has to be sent
 * Return type: void
 */

void signalProducer(pid_t pid) {
	kill(pid, SIGUSR2); 
}

/*
 * FunctionName: waitForProducer
 * Makes consumer process wait for parent process
 * Parameters: process id to which the signal has to be sent
 * Return type: void
 */

void waitForProducer() {
	while (sigflag == 0) {
		// Wait for signal from parent
		sigsuspend(&emptymask);
	}
	sigflag = 0; 
}

/*
 * FunctionName: signalConsumer
 * Send SIGUSR1 signal to consumer or child process
 * Parameters: process id to which the signal has to be sent
 * Return type: void
 */

void signalConsumer(pid_t pid) {
	kill(pid, SIGUSR1); 
}

/*
 * FunctionName: waitForConsumer
 * makes parent or producer to wait for consumer
 * Parameters: process id to which the signal has to be sent
 * Return type: void
 */

void waitForConsumer() {
	while (sigflag == 0) {
		// Wait for signal from from child
		sigsuspend(&emptymask);
	}
	sigflag = 0;
}
