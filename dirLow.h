/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name:: Phillip Davis, Igor Tello Jared Aung, Preet Vithani
* Student IDs:: 923980431, 923043807, 922772159, 923575806
* GitHub-Name:: R3plug
* Group-Name::  Team Kentucky Kernels
* Project:: Basic File System
*
* File:: dirLow.h
*
* Description:: Header file for low level directory functions
*
**************************************************************/
#ifndef DIRLOW
#define DIRLOW

#include "fsFreeSpace.h" 
#include <stdlib.h>
#include <time.h>
#include <string.h>
//#include "fsInit.h"
#include "fsLow.h"
#define MAX_NAME_LENGTH 255

typedef struct ExtentTable ExtentTable;

typedef struct DE{
    ExtentTable mem;//Extent table to hold Directory entry data
    time_t creationTime;
    time_t modificationTime;    // Directory metadata
    time_t lastAccessTime;
    uint32_t size;// size in bytes of directory
    char name[MAX_NAME_LENGTH];//directory name
    int isDir;//signal value for if directory entry is a directory

}DE;

typedef struct ppInfo{
    DE* parent;
    int index;
    char lastElementName[MAX_NAME_LENGTH];
}ppInfo;

//creates a directory with a given number of entries at 
//the provided path
DE* createDir(int numEntries,DE* parent);

//creates a file with given name
DE *createFile(const char *name, DE *parent);

//Takes a path and verifies it is a valid path in the fs
int parsePath(const char* path,ppInfo* info);

//writes a directory to disk
int writeDir(DE* newDir);
DE* loadDir(DE* dir);

DE *getRootDir(void);
DE *getCwd(void);

int findFreeDE(DE *dir);

void setRootDir(DE *root);
void setCwdDir (DE *cwd);

#endif