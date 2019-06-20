#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"    /* For the message struct */
using namespace std;
/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the allocated message queue
 */
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	/* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
 	    so manually or from the code).
	2. Use ftok("keyfile.txt", 'a') in order to generate the key.
	3. Use will use this key in the TODO's below. Use the same key for the queue
	   and the shared memory segment. This also serves to illustrate the difference
 	   between the key and the id used in message queues and shared memory. The key is
	   like the file name and the id is like the file object.  Every System V object
	   on the system has a unique id, but different objects may have the same key.
	*/
	printf("<Line 35> Initializing sender...\n");
	key_t key = ftok("keyfile.txt", 'a');

	/* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
	/* TODO: Attach to the shared memory */
	/* TODO: Attach to the message queue */
	/* Store the IDs and the pointer to the shared memory region in the corresponding function parameters */
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0644 | IPC_CREAT);
	if (shmid == -1) { //get id and create queue if it does not exist
		cout << "<Line 44> Error in shmget, cannot obtain shared memory segment id";
		exit(1);
	}

	sharedMemPtr = shmat(shmid, sharedMemPtr, 0);

	msqid = msgget(key, 0666 | IPC_CREAT);
	if (msqid == -1) {  //connects to message queue and generates id
		perror("<Line 52> Error in msgget, cannot attatch to message queue");
		exit(1);
	}

	printf("<Line 56> DEBUG: msqid(%d)\n", msqid);
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* TODO: Detach from shared memory */
	shmdt(sharedMemPtr);
}

/**
 * The main send function
 * @param fileName - the name of the file
 * @return - the number of bytes sent
 */
unsigned long sendFile(const char* fileName)
{

	/* A buffer to store message we will send to the receiver. */
	struct message sndMsg;
	/* A buffer to store message received from the receiver. */
	struct message rcvMsg;

	sndMsg.mtype = SENDER_DATA_TYPE;

	/* The number of bytes sent */
	unsigned long numBytesSent = 0;

	/* Open the file */
	FILE* fp = fopen(fileName, "r");

	/* Was the file open? */
	if (!fp)
	{
		perror("<Line 95> Error, File not open");
		exit(-1);
	}

	/* Read the whole file */
	while (!feof(fp))
	{
		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and
		 * store them in shared memory.  fread() will return how many bytes it has
		 * actually read. This is important; the last chunk read may be less than
		 * SHARED_MEMORY_CHUNK_SIZE.
		 */
		sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp);

		if(sndMsg.size < 0)
		{
			perror("fread");
			exit(-1);
		}

		printf("<Line 115> Starting sender program...\n");
		sndMsg.mtype = SENDER_DATA_TYPE;
		printf("<Line 117> DEBUG: Size of Message(%d) Message Type(%ld)\n", sndMsg.size, sndMsg.mtype);
		/* TODO: count the number of bytes sent. */
		//documentation source: http://www.cplusplus.com/reference/cstdio/fread/

		/* TODO: Send a message to the receiver telling him that the data is ready
		 * to be read (message of type SENDER_DATA_TYPE).
		 */
		if(msgsnd(msqid, &sndMsg, sizeof(struct message) - sizeof(long), 0) == -1)
		{
			perror("<Line 126> Error, message could not send properly to reciever");
		}

		numBytesSent = sndMsg.size;

		/* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
		 * that he finished saving a chunk of memory.
		 */
		 printf("<Line 134> Hanging Here ...\n");
		if(msgrcv(msqid, &rcvMsg, 0, RECV_DONE_TYPE, 0) == -1 )
		{
			perror("<Line 137> Error, could not properly recieve message from reciever process");
			exit(1);
		}
	}


	/** TODO: once we are out of the above loop, we have finished sending the file.
	  * Lets tell the receiver that we have nothing more to send. We will do this by
	  * sending a message of type SENDER_DATA_TYPE with size field set to 0.
	  */

	sndMsg.size = 0;
	sndMsg.mtype = SENDER_DATA_TYPE;

	printf("<Line 151> DEBUG: Message Size %d Message Type %ld\n", sndMsg.size, sndMsg.mtype);

	if(msgsnd(msqid, &sndMsg, sizeof(struct message) - sizeof(long), 0) == -1){
		perror("<Line 154>Error, could not send message indicating sender is done sending");
		exit(1);
	}

	/* Close the file */
	fclose(fp);
	printf("<Line 160> Closed file.\n");

	return numBytesSent;
}

/**
 * Used to send the name of the file to the receiver
 * @param fileName - the name of the file to send
 */
void sendFileName(const char* thefileName)
{
	/* Get the length of the file name */
	int fileNameSize = strlen(thefileName);

	/* TODO: Make sure the file name does not exceed
	* the maximum buffer size in the fileNameMsg
	* struct. If exceeds, then terminate with an error. */

	if (fileNameSize > MAX_FILE_NAME_SIZE) {
		perror("File Name is too large\n");
		exit(1);
	}

	/* TODO: Create an instance of the struct representing the message
	* containing the name of the file.
	*/
	/* TODO: Set the message type FILE_NAME_TRANSFER_TYPE */
	struct fileNameMsg sendMessageName = { FILE_NAME_TRANSFER_TYPE}; //temorary mtype and size for testing
	cout << "\n188: address of struct: " << &sendMessageName << endl;
	/* TODO: Set the file name in the message */
	//make size of message name dynamic
	strncpy(sendMessageName.fileName, thefileName, fileNameSize);
	cout << "\n192: name of file: " << sendMessageName.fileName << endl;
	/* TODO: Send the message using msgsnd */
	//replaced this parameter sizeof(struct fileNameMsg) - sizeof(long)
	if (msgsnd(msqid, &sendMessageName,fileNameSize, 0) == -1) {
		perror("Error, message could not send properly to reciever");
	}

}


int main(int argc, char** argv)
{

	/* Check the command line arguments */
	if(argc < 2)
	{
		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
		exit(-1);
	}

	/* Connect to shared memory and the message queue */
	init(shmid, msqid, sharedMemPtr);

	/* Send the name of the file */
  sendFileName(argv[1]);

	/* Send the file */
	fprintf(stderr, "The number of bytes sent is %lu\n", sendFile(argv[1]));

	/* Cleanup */
	cleanUp(shmid, msqid, sharedMemPtr);

	return 0;
}
