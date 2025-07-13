/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis Igor Tello Jared Aung Preet Vithani
* Student IDs::923980431
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*	This is the file system interface implementation.
*	This is the implementation of the interface needed 
*	by the driver to interact with your filesystem.
*
**************************************************************/

#include "mfs.h"
#include "dirLow.h"

int fs_mkdir(const char *pathname, mode_t mode){
    ppInfo ppi;

    int retPP = parsePath(pathname,&ppi);

    if(retPP!=0){
        return -1;
    }

    if(ppi.index !=-1){
        return -2;
    }

    DE* newDir = createDir(50,ppi.parent);

    int index = findFreeDE(ppi.parent);

    ppi.parent[index].size=newDir[0].size;
    ppi.parent[index].isDir=newDir[0].isDir;
    ppi.parent[index].creationTime=newDir[0].creationTime;
    ppi.parent[index].lastAccessTime=newDir[0].lastAccessTime;
    ppi.parent[index].modificationTime=newDir[0].creationTime;
    ppi.parent[index].mem = newDir[0].mem;
    strncpy(ppi.parent[index].name,ppi.lastElementName,MAX_NAME_LENGTH);

    if(writeDir(ppi.parent)!=0){
        return -3;  //failed write to save dir
    };
    free(newDir);
    safeFree(ppi.parent);
    return 0;
}

/**
 * Returns an int indicating the position of a free 
 * directory entry in parent.
 * if directory is full it expands it.
 */
int findFreeDE(DE* parent){
    if(parent = NULL){
        return -1;
    }
    if(!isDir(parent)){
        return -1;
    }

    int dirEntries = parent[0].size/sizeof(DE);

    char* emptyDirName ='\0';
    int index;
    for(int index = 0; index<dirEntries;index++){
        if(strcmp(parent[index].name,emptyDirName)==0){
            return index;
        }
    }

    if(expandDirectory(parent)!=0){
        return -1;
    };

    return index++;
}

int expandDirectory(DE* dir){
    VCB* tempVCB =getVCB();
    int blockSize = tempVCB->blockSize;
    int numEntries = 50;
    int memNeeded = numEntries*sizeof(DE);
    int blocksNeeded = (memNeeded+blockSize-1)/blockSize;
    memNeeded = blocksNeeded*blockSize;    //Accounts for allocating memory in blocks
    

    DE* newDir = malloc(memNeeded);//initialize directory array

    int actualEntries = memNeeded/sizeof(DE);//calculate the number of entries that fit in
                                             //allocated memory

    for(int i =2; i<actualEntries; i++){//Mark unused entries
        newDir[i].name[0] ='\0';
    }
    uint32_t blocksAllocated =0;

    Extent* dirMem = allocateFreeBlocks(blocksNeeded,&blocksAllocated);//get memory for directory

    //printf("\nExtent count: %d\n", dirMem->count);

    if(dirMem ==NULL){
        printf("No memory allocated for root dir\n");
        return -1;
    }

    int nextFreeExt = dir->mem.extentCount;

    dir[0].mem.extents[nextFreeExt].count = dirMem->count;
    dir[0].mem.extents[nextFreeExt].block = dirMem->block;
    dir[0].mem.extents[nextFreeExt].used = 1;
    dir[0].mem.extentCount++;
    dir[0].size+=memNeeded;

    
    return 0;
}

/**
 * Check that the dir is not the cwd or root dir
 * then free it
 */
void safeFree(DE* dir){
    if(dir!=getCwd() && dir!=getRootDir()){
        free(dir);
    }

}
