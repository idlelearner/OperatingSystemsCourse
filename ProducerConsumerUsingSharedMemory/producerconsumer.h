#ifndef _producerconsumer_h
#define _producerconsumer_h

/*
 * author       : BhagavathiDhass Thirunavukarasu
 * student ID   : 5078702
 * Csci 5103 Operating System assignment1
 */

/* Including libraries */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

/* Constants */
#define MAX_BUFFER_SIZE 1024
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define SHM_KEY 5156

/* Global Variables */
static volatile sig_atomic_t sigflag; // synchronization flag
static sigset_t emptymask;

/* Structure of the shared memory */
struct shared_mem_struct {
	char buffer[MAX_BUFFER_SIZE]; // buffer for writing and reading data
	int count; // count for length of the data
	double start_time_in_microsec; // start time when the data is written in buffer
	double buffer_time; // time for which the data was in buffer
};

/*Struture of the time of the day value*/
struct timeval time;

/* Declaring functions */
static void signalHandler(int signo);
void initSignals();
void signalProducer(pid_t pid);
void waitForProducer();
void signalConsumer(pid_t pid);
void waitForConsumer();
#endif
