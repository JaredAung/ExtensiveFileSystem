/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name:: Phillip Davis, Igor Tello, Jared Aung,  Preet Vithani 
* Student IDs::	923980431, 923043807, 922772159, 923575806
* GitHub-Name::	R3plug, JaredAung
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
#include "dirLow.h"

#define EXTENT_TABLE_BLOCKS 2


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
	char signature[8]; //unique identifier
	uint32_t blockSize; // size of block
	uint32_t totalBlocks; // total number of blocks in volume
	uint32_t extentTableStart; // block number where extent table start
	uint32_t extentTableBlocks; //number of blocks used by extent table
	uint32_t rootDirStart; //block number where root directory start
	uint32_t rootDirBlocks;//number of blocks used by root directory
	uint32_t freeBlockStart;//block number where free space start

	time_t createTime; //time when volume was created
	time_t lastMountTime; // time when volume was last mounted

}VCB;

Extent *allocateFreeBlocks(uint32_t minExtentLength, uint32_t *extentsAllocated);

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize);

void exitFileSystem ();

DE* getRoot();

#endif