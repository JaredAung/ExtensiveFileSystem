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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mfs.h"
#include "dirLow.h"
#include "fsFreeSpace.h"

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

int fs_delete(char* filename){

    ppInfo ppi;
    
    int parseResult = parsePath(filename, &ppi);
    if(parseResult !=0){
        return -1; // ParsePath failed
    }

    if(ppi.index == -1){
        safeFree(ppi.parent);
        return -1; // File not found
    }

    DE* fileEntry = &ppi.parent[ppi.index];

    // Make sure file is a file, not a directory
    if(fileEntry->isDir){
        safeFree(ppi.parent);
        return -1; // Cannot delete a directory
    }

    // Free file's data blocks
    if(freeBlocks(&fileEntry->mem) != 0){
        safeFree(ppi.parent);
        return -1; // Failed to free blocks
    }

    // Mark directory entry as not used
    memset(fileEntry, 0, sizeof(DE));

    // Write updated directory to disk
    if(writeDir(ppi.parent) != 0){
        safeFree(ppi.parent);
        return -1; // Failed to write directory
    }

    safeFree(ppi.parent);
    return 0;
}

int freeBlocks(ExtentTable* mem){

    if(!mem) return -1;

    for(uint32_t i = 0; i < mem->extentCount; i++){
        if(mem->extents[i].used){
            releaseBlocks(mem->extents[i].block, mem->extents[i].count);
            mem->extents[i].used = 0; // Change extent to not used
        }
    }
    mem->extentCount = 0;
    return 0;
}

// This function will set the current working directory.
int fs_setcwd(char *pathname){
    //We need to allocate memory for return value of parsePath
    ppInfo* ppi    = malloc(sizeof(ppInfo));
    char  *pathCpy = strdup(pathname);
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    //We need to check for errors
    if(ppi->index < 0){
        freePPI(ppi);
        return -1;
    }
    //We need to check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_setcwd:ERROR IN PARSE PATH: %d\n", retVal);
        return retVal;
    }
    
    DE* entry = ppi->parent;
    //We need to make sure last value is a valid directory
    if(!entryIsDir(entry, ppi->index)){
        printf("fs_setcwd: is not a valid path\n");
        freePPI(ppi);
        return -1;
    }
    //We need to load the directory to memory
    DE* cwd = loadDir(entry, ppi->index);
    //We need to set current directory
    freePPI(ppi);
    setCwdDir(cwd);
    //We need to update str value of CWD
    pathCleaner(pathname);
    return 0;
}

//This function gets the current working directory.
char* fs_getcwd(char *pathname, size_t size){
    char* retVal = strncpy(pathname, getCWDStr(), size);
    return retVal;
}

//this function (fs_isFile) returns 1 if it is a file, 0 if it not
int fs_isFile(char * filename){
    //We need to allocate memory for return value of parsePath
    ppInfo* ppi = malloc(sizeof(ppInfo));
    if (ppi == NULL) {
        return -1;
    }
    
    char *pathCpy = strdup(filename);
    if (pathCpy == NULL) {
        free(ppi);
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    //We need to check for errors (does not exist)
    if(ppi->index == -1){
        freePPI(ppi);
        return 0;
    }
    //We need to check for parse errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    
    //We need to check whether it is not a directory
    int isFile = !entryIsDir(ppi->parent, ppi->index);
    freePPI(ppi);
    return isFile;
}

// This Function returns if the directory is file or directory.
int fs_isDir(char * filename){
    // We need to allocate memory for return value of parsePath
    ppInfo* ppi = malloc(sizeof(ppInfo));
    if (ppi == NULL) {
        return -1;
    }
    
    char *pathCpy = strdup(filename);
    if (pathCpy == NULL) {
        freePPI(ppi);
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    //We need to check for errors 
    if (ppi->index == -1) {
        freePPI(ppi);
        return 0;
    }
    
    //We need to check for parse errors
    if (retVal < 0) {
        freePPI(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    
    // We need to check whether it is a directory
    retVal = entryIsDir(ppi->parent, ppi->index);
    freePPI(ppi);
    return retVal;
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


// fs_opendir opens a directory by loading all its DE from disk and return fdDir* 
// handle that can be used to iterate through the entries
fdDir * fs_opendir (const char *pathname){
    ppInfo info;
    // Parse the path into parent directory and target entry
    
    if (parsePath((char*)pathname,&info) != 0){
        return NULL; 
    }

    
    DE* dir;
    // check if the path is root directory
    if(info.index != -1){
        // get the DE at the desired index inside parent directory
        dir = &info.parent[info.index];
    }
    else {
        // root directory
        dir = info.parent;
    }

    // check directory exists and is a valid directory
    if(!dir || !dir->isDir){
        safeFree(info.parent);
        return NULL;
    }

    
    fdDir* dirp = malloc(sizeof(fdDir));
    if (!dirp) return NULL;

    // allocate structure to hold directory entry information
    DirHandle* handle = malloc(sizeof(DirHandle));
    if (!handle){
        free(dirp);
        return NULL;
    }

    // Count the total DEs in all extents
    int totalDEs = 0;
    for (int i = 0; i < dir->mem.extentCount;i++){
        totalDEs += dir->mem.extents[i].count * (BLOCK_SIZE/sizeof(DE));
    }

    // Allocate space to store all the DEs
    handle->entries = malloc(totalDEs * sizeof(DE));
    if (!handle->entries){
        free(handle);
        free(dirp);
        return NULL;
    }

    int currentDE = 0;

    // copies all the directory entires into entries array
    for (int i =0; i < dir->mem.extentCount; i++){
        int startBlock = dir->mem.extents[i].block;
        int numBlocks = dir->mem.extents[i].count;

        for (int j = 0; j < numBlocks; j++){
            void* buffer = malloc(BLOCK_SIZE);
            if(!buffer) continue;

            LBAread(buffer,1,startBlock + j);
            int entriesInBlock = BLOCK_SIZE / sizeof(DE);
            memcpy(&handle->entries[currentDE], buffer, BLOCK_SIZE);
            currentDE += entriesInBlock;

            free(buffer);
        }
    }
    
    // Set metadata
    handle->totalEntries = currentDE;
    handle-> currentIndex = 0;

    dirp->d_reclen = sizeof(fdDir);
    dirp->dirEntryPosition = 0;
    dirp->di = (struct  fs_diriteminfo *) handle;

    safeFree(info.parent);
    return dirp;

}
// read the next entry in directory
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    // validate directory and handle
    if (!dirp || !dirp->di) return NULL;

    // Cast to DirHandle for access
    DirHandle* handle = (DirHandle*)(dirp->di);

    // iterate through all the DEs until a valid entry is found or end is reacted
    // for valid entry, fill the details in diriteminfo structure 
    while(handle->currentIndex < handle->totalEntries){
        DE * entry = &handle->entries[handle->currentIndex++];
        if(entry->name[0] == '\0') continue; // skip empty entries

        struct fs_diriteminfo * info = malloc(sizeof(struct fs_diriteminfo));
        info->d_reclen = sizeof(struct fs_diriteminfo);
        info->fileType = entry->isDir ? FT_DIRECTORY : FT_REGFILE;
        strncpy(info->d_name,entry->name,255);
        info->d_name[255] = '\0';
        return info;

    }
    return NULL;
}


// close the directory and free memory
int fs_closedir(fdDir* dirp) {
    if(!dirp) return -1;
    DirHandle* handle = (DirHandle*)(dirp->di);
    if (handle){
        if (handle->entries) free(handle->entries);
        free(handle);
    }
    free(dirp);
    return 0;
}
