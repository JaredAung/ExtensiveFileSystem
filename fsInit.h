/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name:: Phillip Davis, Igor Tello, Jared Aung,  Preet Vithani 
* Student IDs::	923980431, 923043807, 922772159, 923575806
* GitHub-Name::	R3plug
* Group-Name::	Team Kentucky Kernels
* Project:: Basic File System
*
* File:: fsInit.h
*
* Description:: Header file initialization of system
*
**************************************************************/
#ifndef FSINIT
#define FSINIT

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "fsFreeSpace.h"
//#include "dirLow.h"


#define ROOT_DIRECTORY_BLOCKS 2 // Edit this as needed
#define BLOCK_SIZE 512	//default block size

#define FS_SIGNATURE "MFSv1.0\0"

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

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize);

void exitFileSystem ();


#endif