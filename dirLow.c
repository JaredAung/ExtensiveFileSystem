/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis, Igor Tello, Preet Vithani, Jared Aung
* Student IDs::923980431, 923043807, 923575806, 922772159
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
#include "dirLow.h"
#include "fsInit.h"
#include "mfs.h"
#include <stdio.h>

#define MAX_NAME_LENGTH 255
#define BLOCK_SIZE 512

DE* root =NULL;
static DE *g_rootDir = NULL;
static DE *g_cwdDir  = NULL;

int findInDir(DE* parent,char* token1);
DE* loadDir(DE* dir);
int setRoot();
//DE* getRootDir();

void setRootDir(DE *root) { 
    g_rootDir = root;  
    g_cwdDir = root; 
}  

void setCwdDir (DE *cwd)  { 
    g_cwdDir  = cwd;               
    }

DE *getRootDir(void) { 
    return g_rootDir;
    }

DE *getCwd   (void) { 
    return g_cwdDir;  
    }

DE* createDir(int numEntries,DE* parent){
    int memNeeded = numEntries*sizeof(DE);
    int blocksNeeded = (memNeeded+BLOCK_SIZE-1)/BLOCK_SIZE;
    memNeeded = blocksNeeded*BLOCK_SIZE;    //Accounts for allocating memory in blocks
    //printf("blocksNeeded %d\n", blocksNeeded);

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
    //printf("Extents count for root dir %d", numExtents);
    //printf("Start lcoation root dir %d", newDir[0].mem.extents[0].block);
    
    //if not a new directory write full extent table of data to disk.
    for(int i = 0; i<numExtents; i++){
        //all LBAWrite data stored in the '.' entry of the directory
        LBAwrite(newDir,newDir[0].mem.extents[i].count, newDir[0].mem.extents[i].block);
    }
    

}
/**
 * Takes a file path and an info structure
 * returns 0 on success, -1 on doesnt exist 
 */
int parsePath(const char* pathname,ppInfo* info){

    if(root = NULL){
        setRoot();
    }

    if(pathname == NULL){//empty path invalid
        return -1;
    }

    int pathLength = strlen(pathname);
    if(pathLength <1){
        return -1;
    }

    DE* parent;
    DE* startParent;
    char* savePtr;
    char* token1;
    char* token2;

    char*path = malloc(pathLength+1);
    

    strcpy(path,pathname);
    
    //Sets starting point of path search
    if(path[0]=='/'){
        startParent = getRootDir();
    }
    else{
        startParent = getCwd();
    }

    parent = startParent;

    token1 = strtok_r(path, "/", &savePtr);

    if(token1==NULL){
        if(path[0]=='/'){//passed the root directory
            info->parent = parent;
            info->index = -2;
            info->lastElementName = NULL;
            free(path);
            return 0;
        }
        else{
            free(path);
            return -1;
        }
    }
    while(1){
        int idx = findInDir(parent, token1);

        token2 = strtok_r(NULL,"/",&savePtr);

        if(token2 ==NULL){
            info->parent = parent;
            info->index = idx;
            info->lastElementName = token1;
            free(path);
            return 0;
        }

        if(idx ==-1){
            free(path);
            return -1;
        }

        if(parent[idx].isDir!='1'){
            free(path);
            return -1;
        }

        DE* tempParent = loadDir(&(parent[idx]));

        if(parent!= startParent){
            
            free(parent);
        }
        
        parent = tempParent;

        token1=token2;

    }
}


DE* loadDir(DE* dir){
    if(dir == NULL){
        return NULL;//invalid input
    }
    
    if(dir->isDir!= '1'){
        return NULL;//is not a directory
    }

    DE* tempDir = malloc(dir->size);

    int extInDir = dir->mem.extentCount;
    int index = 0;

    for(int i = 0; i<extInDir;i++){
        int loc =dir->mem.extents[i].block;
        int blocks = dir->mem.extents[i].count;
        LBAread(&(tempDir[index]),blocks,loc);

    }
    
    return tempDir;
    
}


/**
 * Return the index of the token if the token 
 * is in the Directory
 * Returns -1 if the token is not in the directory
 */
int findInDir(DE* parent,char* token1){
    int numEntries = parent[0].size/sizeof(DE);

    for(int i = 0;i<numEntries;i++){
        if(strcmp(parent[i].name,token1)==0){
            return i;
        }

    }
    return -1;

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
		// struct fs_stat* temp = malloc(sizeof(struct fs_stat));
		// fs_stat("/",temp);

		// int block_size =temp->st_blksize;
        int block_size = tempVCB->blockSize;
        free(tempVCB);
		
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


