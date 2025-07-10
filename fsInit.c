/**************************************************************
 * Class::  CSC-415-02 Summer 2024
 * Name::  Preet Vithani, Igor Tello
 * Student IDs::	923575806, 923043807
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
#include "fsInit.h"
#include "dirLow.h"

#define EXTENT_TABLE_BLOCKS 2
#define ROOT_DIRECTORY_BLOCKS 10 // Edit this as needed
#define FS_SIGNATURE "MFSv1.0\0"
#define BLOCK_SIZE 512

Extent *allocateFreeBlocks(uint32_t minExtentLength, uint32_t *extentsAllocated)
{

	// Load extent table from disk
	ExtentTable *table = (ExtentTable *)malloc(EXTENT_TABLE_BLOCKS * BLOCK_SIZE);
	if (!table)
	{
		perror("Failed to allocate memory for extent table");
		return NULL;
	}

	if (LBAread(table, EXTENT_TABLE_BLOCKS, 1) != EXTENT_TABLE_BLOCKS)
	{
		perror("Failed to read extent table from disk");
		free(table);
		return NULL;
	}

	// lets now allocate spcae to store the extends we will be returning
	Extent *allocatedExtents = calloc(EXTENT_TABLE_BLOCKS, sizeof(Extent));
	if (!allocatedExtents)
	{
		perror("Failed to allocate memory for result extents");
		return NULL;
	}

	uint32_t resultIndex = 0;

	for (int i = 0; i < table->extentCount; i++)
	{
		Extent *extent = &table->extents[i];

		// we need to look for free extends that meet the size
		if (extent->used == 0 && extent->count >= minExtentLength)
		{
			// we add to the result array
			allocatedExtents[resultIndex++] = (Extent){
				.block = extent->block,
				.count = extent->count,
				.used = 1};

			// lets remove the extent from the table
			for (int j = i; j < table->extentCount - 1; j++)
			{
				table->extents[j] = table->extents[j + 1];
			}
			table->extentCount--;
			i--; // lets check the current index after shifting
		}
	}

	if (resultIndex == 0)
	{
		// if no extends is allocated
		//  lets free the extend and return NULL
		*extentsAllocated = 0;
		free(allocatedExtents);
		free(table);
		return NULL;
	}

	LBAwrite(table, EXTENT_TABLE_BLOCKS, 1);

	// lets return the allocated extents and count
	*extentsAllocated = resultIndex;
	free(table);
	return allocatedExtents;
}

int initFreeSpace(uint64_t numberOfBlocks, uint64_t blockSize)
{

	ExtentTable *extentTable = (ExtentTable *)calloc(1, EXTENT_TABLE_BLOCKS * blockSize);
	if (extentTable == NULL)
	{
		perror("Failed to allocate memory for extent table");
		return -1;
	}

	int index = 0;

	// Block 0 for VCB
	extentTable->extents[index++] = (Extent){.block = 0, .count = 1, .used = 1};

	// Extent table blocks
	extentTable->extents[index++] = (Extent){.block = 1, .count = EXTENT_TABLE_BLOCKS, .used = 1};

	// Root directory blocks
	extentTable->extents[index++] = (Extent){
		.block = 1 + EXTENT_TABLE_BLOCKS,
		.count = ROOT_DIRECTORY_BLOCKS,
		.used = 1};

	uint32_t freeSpaceBlocks = 1 + EXTENT_TABLE_BLOCKS + ROOT_DIRECTORY_BLOCKS;
	uint32_t remaining = numberOfBlocks - freeSpaceBlocks;
	// Rest of blocks are free space
	extentTable->extents[index++] = (Extent){.block = freeSpaceBlocks, .count = remaining, .used = 0};

	extentTable->extentCount = index;

	LBAwrite(extentTable, EXTENT_TABLE_BLOCKS, 1);

	free(extentTable);

	return 0;
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Kentucky Kernels\n");
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	VCB *vcb = (VCB *)calloc(1, blockSize);
	if (!vcb)
	{
		perror("VCB Allocation failed");
		return -1;
	}

	if (initFreeSpace(numberOfBlocks, BLOCK_SIZE) != 0)
	{
		printf("Error allocating free space\n");
		free(vcb);
		return -1;
	}

	DE *root = createDir(50, NULL);

	strncpy(vcb->signature, FS_SIGNATURE, 8);
	vcb->blockSize = blockSize;
	vcb->totalBlocks = numberOfBlocks;
	vcb->extentTableStart = 1;
	vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;
	vcb->rootDirStart = root->mem.extents[0].block;
	vcb->rootDirBlocks = root->mem.extents[0].count;
	vcb->freeBlockStart = 1;
	vcb->createTime = time(NULL);
	vcb->lastMountTime = time(NULL);

	if(1!=LBAwrite(vcb, 1, 0)){
		printf("VCB write fail\n");
		return -1;
	};

	printf("blockSize: %d total blocks: %d Extent table start: %d extent table blocks: %d \n", vcb->blockSize,vcb->totalBlocks,vcb->extentTableStart,vcb->extentTableBlocks);
	printf("rootDir start: %d root dir blocks %d free block start %d create %d mount %d\n", vcb->rootDirStart, vcb->rootDirBlocks, vcb->freeBlockStart, vcb->createTime, vcb->lastMountTime);

	free(vcb);
	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}