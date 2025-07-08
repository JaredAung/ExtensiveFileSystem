/**************************************************************
* Class::  CSC-415-02 Summer 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"

#define MAX_EXTENTS 1024
#define EXTENT_TABLE_BLOCKS 25
#define ROOT_DIRECTORY_BLOCKS 10 // Edit this as needed


typedef struct Extent{
	uint32_t block; //block location
	uint32_t count; //number of blocks at this location
	int used; // 1 if used, 0 if free
} Extent;

typedef struct ExtentTable{
	Extent extents[MAX_EXTENTS]; 
	uint32_t extentCount; //number of extents in extent table.
} ExtentTable;


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	// VCB code here

	ExtentTable *extentTable = (ExtentTable *)calloc(1, EXTENT_TABLE_BLOCKS * blockSize);
	if(extentTable == NULL){
		perror("Failed to allocate memory for extent table");
		//free(vcb); 
		return -1;
	}

	int index = 0;

	// Block 0 for VCB
	extentTable->extents[index++] = (Extent){.block = 0, .count = 1, .used = 1};

	// Extent table blocks
	extentTable->extents[index++] = (Extent){.block = 1, .count = EXTENT_TABLE_BLOCKS, .used = 1};

	// Root directory blocks
	extentTable->extents[index++] = (Extent){.block = 1 + EXTENT_TABLE_BLOCKS, .count = ROOT_DIRECTORY_BLOCKS, .used = 1};

	uint32_t freeSpaceBlocks = 1 + EXTENT_TABLE_BLOCKS + ROOT_DIRECTORY_BLOCKS;
	uint32_t remaining = numberOfBlocks - freeSpaceBlocks;
	// Rest of blocks are free space
	extentTable->extents[index++] = (Extent){.block = freeSpaceBlocks, .count = remaining, .used = 0};

	extentTable->extentCount = index;

	LBAwrite (extentTable, EXTENT_TABLE_BLOCKS, 1);

	// vcb->extentTableStart = 1;
	// vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}