/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name:: Phillip Davis, Igor Tello Jared Aung Preet Vithani
* Student IDs::	923980431, 923043807
* GitHub-Name::	R3plug
* Group-Name::	Team Kentucky Kernels
* Project:: Basic File System
*
* File:: dirLow.h
*
* Description:: Header file for low level directory functions
*
**************************************************************/
#ifndef FSINIT
#define FSINIT

#include <stdlib.h>

#define EXTENT_TABLE_BLOCKS 25


typedef struct Extent{
	uint32_t block; //block location
	uint32_t count; //number of blocks at this location
	int used; // 1 if used, 0 if free
} Extent;

typedef struct ExtentTable{
	Extent extents[EXTENT_TABLE_BLOCKS]; 
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

Extent *allocateFreeBlocks(uint32_t minExtentLength, uint32_t *extentsAllocated);

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize);

void exitFileSystem ();

#endif