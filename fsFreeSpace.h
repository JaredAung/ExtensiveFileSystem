/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name:: Phillip Davis, Igor Tello Jared Aung, Preet Vithani
* Student IDs:: 923980431, 923043807, 922772159, 923575806
* GitHub-Name:: R3plug
* Group-Name::  Team Kentucky Kernels
* Project:: Basic File System
*
* File:: fsFreeSpace.h
*
* Description:: Header file for Free Space manager
*
**************************************************************/

#ifndef FSFREESPACE_H
#define FSFREESPACE_H

#define EXTENT_TABLE_BLOCKS 25

#include <stdint.h>
#include "fsInit.h"

typedef struct Extent{
	uint32_t block; //block location
	uint32_t count; //number of blocks at this location
	int used; // 1 if used, 0 if free
} Extent;

typedef struct ExtentTable{
	Extent extents[EXTENT_TABLE_BLOCKS]; 
	uint32_t extentCount; //number of extents in extent table.
} ExtentTable;

Extent *allocateFreeBlocks(uint32_t minExtentLength, uint32_t *extentsAllocated);

int releaseBlocks(uint32_t startBlock, uint32_t count);

int initFreeSpace(uint64_t numberOfBlocks, uint64_t blockSize);

#endif
