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

    return 0;
}

/**
 * Returns an int indicating the position of a free 
 * directory entry in parent.
 * if directory is full it expands it.
 */
int findFreeDE(DE* parent){

}
