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

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "fsInit.h"
#include "fsLow.h"

#define MAX_NAME_LENGTH 255


typedef struct{
    ExtentTable mem;//Extent table to hold Directory entry data
    time_t creationTime;
    time_t modificationTime;    // Directory metadata
    time_t lastAccessTime;
    uint32_t size;// size in bytes of directory
    char name[MAX_NAME_LENGTH];//directory name
    char isDir;//signal value for if directory entry is a directory

}DE;

//creates a directory with a given number of entries at 
//the provided path
DE* createDir(int numEntries,DE* parent);


int writeDir(DE* newDir);//writes a directory to disk

#endif