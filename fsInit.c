/**************************************************************
* Class::  CSC-415-02 Summer 2024
* Name::  , Igor Tello
* Student IDs::	, 923043807
* GitHub-Name::
* Group-Name:: Team Kentucky Kernels
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

int allocateFreeBlock(ExtentTable *extentTable) {
    for (int i = 0; i < extentTable->extentCount; i++) {
        Extent *extent = &extentTable->extents[i];

        // We need to look at the free extents
        if (extent->used == 0 && extent->count > 0) {
            int allocatedBlock = extent->block;

            // lets update out extent
            extent->block++;
            extent->count--;

            // We need to remove the extent from the table
			// only if its empty
            if (extent->count == 0) {
                for (int j = i; j < extentTable->extentCount - 1; j++) {
                    extentTable->extents[j] = extentTable->extents[j + 1];
                }
                extentTable->extentCount--;
            }
            return allocatedBlock;
        }
    }
    return -1; // if no space is left
}


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

	int success = allocateFreeBlock(extentTable);
	if (success == -1) {
		printf("Failed to allocate block!\n");
	} else {
		printf("Allocated free block: %d\n", success);
	}

	LBAwrite (extentTable, EXTENT_TABLE_BLOCKS, 1);

	// vcb->extentTableStart = 1;
	// vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;

	free(extentTable);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}