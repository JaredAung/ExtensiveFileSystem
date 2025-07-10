/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis
* Student IDs::923980431
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: dirLow.h
*
* Description:: Header file for low level directory functions
*
**************************************************************/
#ifndef DIRLOW
#define DIRLOW

#include <stdlib.h>
#include <time.h>
#include "fsInit.h"
#include "fsLow.h"

extern MAX_NAME_LENGTH;
extern BLOCK_SIZE; 

typedef struct{
    ExtentTable mem;
    time_t creationTime;
    time_t modificationTime;
    time_t lastAccessTime;
    uint32_t size;
    char name[MAX_NAME_LENGTH];
    char isDir;

}DE;


DE* createDir(int numEntries,DE* parent);

int writeDir(DE* newDir);

#endif