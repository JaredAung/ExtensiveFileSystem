/**************************************************************
 * Class::  CSC-415-02 Summer 2024
 * Name::  Preet Vithani, Igor Tello, Phillip Davis, Jared Aung
 * Student IDs::	923575806, 923043807, 923980431, 922772159
 * GitHub-Name:: R3plug
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

DE* root;


int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	//printf("Kentucky Kernels\n");
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	// allocate Volume Control Block
	VCB *vcb = (VCB *)calloc(1, blockSize);
	if (!vcb) // check allocation was successful
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

	// create Root Directory containing 50 directory entires
	DE *root = createDir(50, NULL);

	//Fill VCB
	strncpy(vcb->signature, FS_SIGNATURE, 8);
	vcb->blockSize = blockSize;
	vcb->totalBlocks = numberOfBlocks;
	vcb->extentTableStart = 1;
	vcb->extentTableBlocks = EXTENT_TABLE_BLOCKS;
	vcb->rootDirStart = root->mem.extents[0].block;
	vcb->rootDirBlocks = root->mem.extents[0].count;
	vcb->freeBlockStart = 1;
	vcb->createTime = time(NULL); //current time
	vcb->lastMountTime = time(NULL); //current time

	// write VCB to block
	if(1!=LBAwrite(vcb, 1, 0)){
		printf("VCB write fail\n");
		return -1;
	};

	

	//printf("blockSize: %d total blocks: %d Extent table start: %d extent table blocks: %d \n", vcb->blockSize,vcb->totalBlocks,vcb->extentTableStart,vcb->extentTableBlocks);
	//printf("rootDir start: %d root dir blocks %d free block start %d create %ld mount %ld\n", vcb->rootDirStart, vcb->rootDirBlocks, vcb->freeBlockStart, vcb->createTime, vcb->lastMountTime);

	setRoot();//Sets a variable to hold the root while running
	free(vcb); // free allocated memory
	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}

int setRoot(){
		//Load VCB to get root info
		VCB* tempVCB =malloc(sizeof(VCB));
		if(LBAread(tempVCB, 1,0)!= 1){
			return -1; //read failed
		};

		int rootLoc = tempVCB->rootDirStart;
		int rootSize = tempVCB->rootDirBlocks;

		//Get block size info
		struct fs_stat* temp = malloc(sizeof(struct fs_stat));
		fs_stat("/",temp);

		int block_size =temp->st_blksize;
		
		//allocate memory for global root
		root = malloc(block_size*rootSize);

		if(LBAread(root,rootSize,rootLoc)!=rootSize){
			return -1;
		};
		return 0;

	}

	DE* getRoot(){
		return root;
	}