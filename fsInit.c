/**************************************************************
* Class::  CSC-415-02 Summer 2024
* Name::  Preet Vithani, Igor Tello
* Student IDs::	923575806, 923043807
* GitHub-Name:: preet56
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
#define FS_SIGNATURE "MFSv1.0\0"


typedef struct Extent{
	uint32_t block; //block location
	uint32_t count; //number of blocks at this location
	int used; // 1 if used, 0 if free
} Extent;

typedef struct ExtentTable{
	Extent extents[MAX_EXTENTS]; 
	uint32_t extentCount; //number of extents in extent table.
} ExtentTable;

typedef struct VolumeControlBlock{
	char signature[8];
	uint32_t blockSize;
	uint32_t totalBlocks;
	uint32_t extentTableStart;
	uint32_t extentTableBlocks;
	uint32_t rootDirStart;
	uint32_t rootDirBlocks;
	uint32_t freeBlockStart;

	time_t createTime;
	time_t lastMountTime;

}VCB;

Extent *allocateFreeBlocks(ExtentTable *extentTable, 
		uint32_t minExtentLength, uint32_t *extentsAllocated) {

	// lets now allocate spcae to store the extends we will be returning
    Extent *allocatedExtents = calloc(MAX_EXTENTS, sizeof(Extent));
    if (!allocatedExtents) {
        perror("Failed to allocate memory for result extents");
        return NULL;
    }

    uint32_t resultIndex = 0;

    for (int i = 0; i < extentTable->extentCount; i++) {
        Extent *extent = &extentTable->extents[i];

		// we need to look for free extends that meet the size
        if (extent->used == 0 && extent->count >= minExtentLength) {
            // we add to the result array
            allocatedExtents[resultIndex++] = (Extent){
                .block = extent->block,
                .count = extent->count,
                .used = 1
            };

            // lets remove the extent from the table
            for (int j = i; j < extentTable->extentCount - 1; j++) {
                extentTable->extents[j] = extentTable->extents[j + 1];
            }
            extentTable->extentCount--;
            i--;  // lets check the current index after shifting
        }
    }

    if (resultIndex == 0) {
		//if no extends is allocated 
		// lets free the extend and return NULL
        free(allocatedExtents);
        *extentsAllocated = 0;
        return NULL;
    }

    // lets return the allocated extents and count
    *extentsAllocated = resultIndex;
    return allocatedExtents;
}



int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	// VCB SETUP

	VCB* vcb = (VCB *)calloc(1,blockSize);
	if(!vcb){
		perror("VCB Allocation failed");
		return -1;
	}

	strncpy(vcb->signature,FS_SIGNATURE,8);
	vcb->blockSize = blockSize;
	vcb->totalBlocks = numberOfBlocks;
	vcb->extentTableStart = 1;
	vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;
	vcb->rootDirStart = 1 + EXTENT_TABLE_BLOCKS;
	vcb->rootDirBlocks = ROOT_DIRECTORY_BLOCKS;
	vcb->freeBlockStart = 1 + EXTENT_TABLE_BLOCKS + ROOT_DIRECTORY_BLOCKS;
	vcb->createTime = time(NULL);
	vcb->lastMountTime = time(NULL);

	LBAwrite(vcb,1,0);

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

	uint32_t allocatedCount = 0;
	Extent *allocated = allocateFreeBlocks(extentTable, 1, &allocatedCount);

	if (allocated == NULL) {
		printf("No free extents available.\n");
	} else {
		for (uint32_t i = 0; i < allocatedCount; i++) {
			printf("Allocated: Block %u, Count %u\n", allocated[i].block, allocated[i].count);

			// we need to add the used extents in extent table
			extentTable->extents[extentTable->extentCount++] = allocated[i];
		}
		free(allocated);  // freeup
	}

	LBAwrite (extentTable, EXTENT_TABLE_BLOCKS, 1);

	// vcb->extentTableStart = 1;
	// vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;

	free(extentTable);
	free(vcb);
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}