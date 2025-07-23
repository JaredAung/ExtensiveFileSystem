/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis Igor Tello Jared Aung Preet Vithani
* Student IDs::923980431, 922772159, 923575806, 923043807
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "dirLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	char * buff;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	off_t filePosition; // current offset in file (could use uint64_t ?)
	uint64_t fileSize;	// total size of file, updated from b_write
	DE *fi;			// pointer to file's directory entry
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buff = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buff == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags){
    b_io_fd returnFd;
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd < 0) return -1; //No available FCB

	ppInfo info;
	if(parsePath(filename, &info) != 0) return -1; 

	DE *fileEntry = &info.parent->mem.extents[info.index];
	if(fileEntry->isDir) return -1; 	// can't open a directory  

	fcbArray[returnFd].buff = malloc(B_CHUNK_SIZE);
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].filePosition = 0;
	fcbArray[returnFd].fi = fileEntry;
	
	return (returnFd);
}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	b_fcb *fcb = &fcbArray[fd];
	if(fcb->buff ==NULL || fcb->fi == NULL){
		return -1;
	}	

	off_t newPosition;
	// Calculate file position based on whence
	switch(whence) {
		case SEEK_SET:
			newPosition = offset;
			break;
		case SEEK_CUR:
			newPosition = fcb->filePosition +offset;
			break;
		case SEEK_END:
			newPosition = fcb->fileSize + offset;
			break;
		default:
			return -1;

	}	

	// check bounds to ensure newPosition is valid
	if (newPosition < 0 || newPosition > fcb->fileSize){
		return -1;
	}

	// set new position and invalidate buffer
	fcb->filePosition = newPosition;
	fcb->index = B_CHUNK_SIZE;
	fcb->buflen = 0;
		
	return newPosition;
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	b_fcb *fcb = &fcbArray[fd];
	if(fcb->buff ==NULL || fcb->fi == NULL){
		return -1;
	}	
	
	int written = 0;

	while (written < count) {
		int spaceRemain = B_CHUNK_SIZE - fcb->index;
		int bytesToBuffer = (count - written < spaceRemain) ? (count -written) : spaceRemain;

		// Copy data from input buffer to internal buffer
		memcpy(fcb->buff + fcb->index, buffer + written, bytesToBuffer);
		fcb->index += bytesToBuffer;
		written += bytesToBuffer;

		// when the buffer is filled, write to disk
		if(fcb->index == B_CHUNK_SIZE) {
			uint64_t blockOffset = fcb->filePosition / B_CHUNK_SIZE;
			uint64_t writeResult = LBAwrite(fcb->buff,1, fcb->fi->mem.extents->block + blockOffset);

			if(writeResult != 1) return -1;

			fcb->filePosition += B_CHUNK_SIZE;
			fcb->index = 0;
		}
	}
	if(fcb->filePosition + fcb->index > fcb->fileSize){
			fcb->fileSize = fcb->filePosition + fcb->index;
		}
		
	return written; //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	if(fcbArray[fd].fi == NULL) return -1;

	b_fcb *fcb = &fcbArray[fd];

	// Check for end of file
	if(fcb->filePosition >= fcb->fileSize){
		return 0;
	}

	int bytesLeft = fcb->fileSize - fcb->filePosition;
	int bytesToCopy = (count <bytesLeft) ? count : bytesLeft;
	int totalCopied = 0;

	// Copy remaining data in buffer
	if(fcb->index < fcb->buflen){
		int remaining = fcb->buflen - fcb->index;
		int copySize = (bytesToCopy < remaining) ? bytesToCopy : remaining;

		memcpy(buffer, fcb->buff + fcb->index, copySize);

		fcb->index += copySize;
		fcb->filePosition += copySize;
		bytesToCopy -= copySize;
		totalCopied += copySize;
		buffer += copySize;
	}

	// Read full blocks directly into buffer
	if(bytesToCopy >= B_CHUNK_SIZE){

		int fullBlocks = bytesToCopy / B_CHUNK_SIZE;
		uint64_t blockOffset = fcb->filePosition / B_CHUNK_SIZE;

		uint64_t blocksRead = LBAread(buffer, fullBlocks, fcb->fi->mem.extents->block + blockOffset);
		if(blocksRead != fullBlocks){
			return totalCopied; // Partial read
		}

		int bytesRead = fullBlocks * B_CHUNK_SIZE;
		fcb->filePosition += bytesRead;
		bytesToCopy -= bytesRead;
		totalCopied += bytesRead;
		buffer += bytesRead;

		fcb->index = 0;
		fcb->buflen = 0;
	}

	// Load next block into buffer and copy remaining bytes
	if(bytesToCopy > 0){
		uint64_t blockOffset = fcb->filePosition / B_CHUNK_SIZE;
		uint64_t result = LBAread(fcb->buff, 1, fcb->fi->mem.extents->block + blockOffset);

		if(result != 1){
			return totalCopied; // Error or Partial read
		}

		fcb->buflen = B_CHUNK_SIZE;
		fcb->index = 0;

		int offset = fcb->filePosition % B_CHUNK_SIZE;
		int available = B_CHUNK_SIZE - offset;
		int copySize = (bytesToCopy < available) ? bytesToCopy : available;

		
		memcpy(buffer, fcb->buff + offset, copySize);

		fcb->index = offset + copySize;
		fcb->filePosition += copySize;
		totalCopied += copySize;
	}
		
	return totalCopied;
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		if(startup == 0) return 0;

		if(fd < 0 || fd >= MAXFCBS) return -1;

		b_fcb *fcb = &fcbArray[fd];

		if(fcb->buff == NULL || fcb->fi == NULL) {
			return -1;
		}
		// flush write buffer if there are unwritten data
		if(fcb->index > 0){
			uint64_t blockOffset = fcb->filePosition / B_CHUNK_SIZE;
			uint64_t writeResult = LBAwrite(fcb->buff,1,fcb->fi->mem.extents->block + blockOffset);
			if (writeResult != 1) return -1;

			fcb->filePosition += fcb->index;

			if (fcb->filePosition > fcb->fileSize){
				fcb->fileSize = fcb->filePosition;
			}
		}

		// free buffer
		free(fcb->buff);
		fcb->buff = NULL;

		// Clear FCB fields
		fcb->index = 0;
		fcb->buflen = 0;
		fcb->filePosition = 0;
		fcb->fileSize = 0;
		fcb->fi = NULL;

		return 0;

	}
