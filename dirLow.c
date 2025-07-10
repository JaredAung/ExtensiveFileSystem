/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis
* Student IDs::923980431
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: dirLow.c
*
* Description::Implementation low low level directory functions
* including creating a directory and helper functions
*
**************************************************************/

#include <stdint.h>
#include <time.h>
#include "fsLow.h"
#include "dirLow.h"
#include <stdio.h>

#define MAX_NAME_LENGTH 255
#define BLOCK_SIZE 512


DE* createDir(int numEntries,DE* parent){
    int memNeeded = numEntries*sizeof(DE);
    int blocksNeeded = (memNeeded+BLOCK_SIZE-1)/BLOCK_SIZE;
    memNeeded = blocksNeeded*BLOCK_SIZE;    //Accounts for allocating memory in blocks
    printf("blocksNeeded %d\n", blocksNeeded);

    DE* newDir = malloc(memNeeded);//initialize directory array

    int actualEntries = memNeeded/sizeof(DE);//calculate the number of entries that fit in
                                             //allocated memory

    for(int i =2; i<actualEntries; i++){//Mark unused entries
        newDir[i].name[0] ='\0';
    }
    uint32_t blocksAllocated =0;

    Extent* dirMem = allocateFreeBlocks(blocksNeeded,&blocksAllocated);//get memory for directory

    printf("\nExtent count: %d\n", dirMem->count);
    
    if(dirMem ==NULL){
        printf("No memory allocated for root dir\n");
        return NULL;
    }

    time_t initTime = time(NULL);

    strcpy(newDir[0].name,".");

    newDir[0].size =actualEntries* sizeof(DE); //

    newDir[0].mem.extents[0] =*dirMem; //assign directory memory to extent table to '.' entry
    newDir[0].mem.extentCount=1;

    newDir[0].isDir = '1';  //Sentinel value of 1 is True

    newDir[0].creationTime = initTime;
    newDir[0].modificationTime = initTime;
    newDir[0].lastAccessTime = initTime;

    if(parent==NULL){
        parent = newDir;
    }

    int extentFileCount = newDir[1].mem.extentCount;//size of extent table

    strcpy(newDir[1].name,"..");

    newDir[1].size = parent[0].size;

    for(int i = 0; i<extentFileCount; i++){//copy file extent of parent Dir to ".."
        newDir[1].mem.extents[i] = parent[0].mem.extents[i];
    }

    newDir[1].isDir = parent[0].isDir;

    newDir[1].creationTime = parent[0].creationTime;
    newDir[1].modificationTime = parent[0].creationTime;
    newDir[1].lastAccessTime = parent[0].lastAccessTime;

    if(writeDir(newDir)==-1){//returns NULL if the directory fails to write
        return NULL;
    }


    return newDir;


}

int writeDir(DE* newDir){
    int numExtents = newDir[0].mem.extentCount; //Number of extents that will need to be written
    printf("Extents count for root dir %d", numExtents);
    printf("Start lcoation root dir %d", newDir[0].mem.extents[0].block);
    
    //if not a new directory write full extent table of data to disk.
    for(int i = 0; i<numExtents; i++){
        //all LBAWrite data stored in the '.' entry of the directory
        LBAwrite(newDir,newDir[0].mem.extents[i].count, newDir[0].mem.extents[i].block);
    }
    

}