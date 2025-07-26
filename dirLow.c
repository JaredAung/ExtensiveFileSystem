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
#include "fsFreeSpace.h"

#define MAX_NAME_LENGTH 255
#define BLOCK_SIZE 512


static DE *g_rootDir = NULL;
static DE *g_cwdDir = NULL;

int findInDir(DE *parent, char *token1);
DE *loadDir(DE *dir);
int setRoot();
DE* getRoot();

void setRootDir(DE *root)
{
    g_rootDir = root;
    g_cwdDir = root;
}

void setCwdDir(DE *cwd)
{
    g_cwdDir = cwd;
}

DE *getRootDir(void)
{
    return g_rootDir;
}

DE *getCwd(void)
{
    return g_cwdDir;
}

DE *createDir(int numEntries, DE *parent)
{
    int memNeeded = numEntries * sizeof(DE);
    int blocksNeeded = (memNeeded + BLOCK_SIZE - 1) / BLOCK_SIZE;
    //memNeeded = blocksNeeded; // Accounts for allocating memory in blocks
    printf("blocksNeeded %d\n", blocksNeeded);

    int actualEntries = memNeeded / sizeof(DE); // calculate the number of entries that fit in
                                                // allocated memory

    DE *newDir = calloc(actualEntries,sizeof(DE)); // initialize directory array

    

    for (int i = 2; i < actualEntries; i++)
    { // Mark unused entries
        newDir[i].name[0] = '\0';
    }
    uint32_t blocksAllocated = 0;

    Extent *dirMem = allocateFreeBlocks(blocksNeeded, &blocksAllocated); // get memory for directory

    printf("Dir block:%d, Dir count: %d,Dir used:%d\n",dirMem->block,dirMem->count,dirMem->used);
    // printf("\nExtent count: %d\n", dirMem->count);

    if (dirMem == NULL)
    {
        printf("No memory allocated for root dir\n");
        return NULL;
    }

    time_t initTime = time(NULL);

    strcpy(newDir[0].name, ".");

    newDir[0].size = blocksNeeded*BLOCK_SIZE; //

    newDir[0].mem.extents[0].block = dirMem->block; // assign directory memory to extent table to '.' entry
    newDir[0].mem.extents[0].count = dirMem->count;
    newDir[0].mem.extents[0].used = dirMem->used;

    newDir[0].mem.extentCount = 1;

    newDir[0].isDir = 1; // Sentinel value of 1 is True

    newDir[0].creationTime = initTime;
    newDir[0].modificationTime = initTime;
    newDir[0].lastAccessTime = initTime;
    newDir[0].entryCount = actualEntries;

    //if root dir set self to parent
    if (parent == NULL)
    {
        parent = newDir;
    }

    int extentFileCount = newDir[0].mem.extentCount; // size of extent table

    strcpy(newDir[1].name, "..");

    newDir[1].size = parent[0].size;

    for (int i = 0; i < extentFileCount; i++)
    { // copy file extent of parent Dir to ".."
        newDir[1].mem.extents[i].block = parent[0].mem.extents[i].block;
        newDir[1].mem.extents[i].count = parent[0].mem.extents[i].count;
        newDir[1].mem.extents[i].used = parent[0].mem.extents[i].used;
    }

    newDir[1].isDir = parent[0].isDir;

    newDir[1].creationTime = parent[0].creationTime;
    newDir[1].modificationTime = parent[0].creationTime;
    newDir[1].lastAccessTime = parent[0].lastAccessTime;

    if (writeDir(newDir) == -1)
    { // returns NULL if the directory fails to write
        printf("Dir failed to write\n");
        return NULL;
    }

    return newDir;
}

int writeDir(DE *newDir)
{
    int numExtents = newDir[0].mem.extentCount; // Number of extents that will need to be written
    // printf("Extents count for root dir %d", numExtents);
    // printf("Start lcoation root dir %d", newDir[0].mem.extents[0].block);

    // if not a new directory write full extent table of data to disk.

    int offset = 0;


    for (int i = 0; i < numExtents; i++)
    {
        char* start = (char*)newDir +offset;
        // all LBAWrite data stored in the '.' entry of the directory
        uint64_t result = LBAwrite(start, newDir[0].mem.extents[i].count, newDir[0].mem.extents[i].block);
        if(result != newDir[0].mem.extents[i].count){
            printf("LBA write failed in writeDir\n");
            return -1;
        }
        offset+=newDir[0].mem.extents[i].count*BLOCK_SIZE;
    }

    return 0;
}
/**
 * Takes a file path and an info structure
 * returns 0 on success, -1 on doesnt exist
 */
int parsePath(const char *pathname, ppInfo *info)
{

    if (pathname == NULL)
    { // empty path invalid
        return -1;
    }

    int pathLength = strlen(pathname);
    if (pathLength < 1)
    {
        return -1;
    }

    DE *parent;
    DE *startParent;
    char *savePtr;
    char *token1;
    char *token2;

    char *path = malloc(pathLength + 1);

    strcpy(path, pathname);

    // Sets starting point of path search
    if (path[0] == '/')
    {
        startParent = getRootDir();
    }
    else
    {
        startParent = getCwd();
    }

    parent = startParent;

    token1 = strtok_r(path, "/", &savePtr);

    if (token1 == NULL)
    {
        if (path[0] == '/')
        { // passed the root directory
            info->parent = parent;
            info->index = -2;
            info->lastElementName[0] = '\0';
            free(path);
            return 0;
        }
        else
        {
            free(path);
            return -1;
        }
    }
    while (1)
    {
        int idx = findInDir(parent, token1);
        printf("Idx: %d\n", idx);

        token2 = strtok_r(NULL, "/", &savePtr);

        if (token2 == NULL)
        {
            info->parent = parent;
            info->index = idx;
            strcpy(info->lastElementName, token1);
            printf("in oparsePath parent.mem.extentCount: %d\n",parent[idx].mem.extentCount);
            free(path);
            return 0;
        }

        if (idx == -1)
        {
            free(path);
            return -1;
        }

        if (parent[idx].isDir != 1)
        {
            free(path);
            return -1;
        }

        DE *tempParent = loadDir(&(parent[idx]));

        if (parent != startParent)
        {

            free(parent);
        }

        parent = tempParent;

        token1 = token2;
    }
}

DE *loadDir(DE *dir)
{
    if (dir == NULL)
    {
        printf("A NULL directory was passed to function\n");
        return NULL; // invalid input
    }

    if (dir->isDir != 1)
    {
        printf("passed dir not a directory\n");
        return NULL; // is not a directory
    }

    printf("dir->size: %d\n", dir->size);

    int memNeeded = 0;

    for(int i = 0; i<dir->mem.extentCount;i++){
        memNeeded+=dir->mem.extents[i].count*BLOCK_SIZE;
    }

    DE *tempDir = malloc(memNeeded);
    if(tempDir == NULL){
        printf("Malloc failed in loadDir\n");
        return NULL;
    }

    int extInDir = dir->mem.extentCount;
    printf("Extents in dir: %d",extInDir);
    int byteOffset = 0;

    for (int i = 0; i < extInDir; i++)
    {
        int loc = dir->mem.extents[i].block;
        int blocks = dir->mem.extents[i].count;
        int bytesToRead = blocks * BLOCK_SIZE;
        printf("Bytes to Read: %d\n",bytesToRead);

        
        LBAread(tempDir+byteOffset, blocks, loc);
        byteOffset += bytesToRead;
    }
    printf("ByteOffset %d\n",byteOffset);
    printf("DirSize: %d\n",dir->size);
    if(byteOffset!=dir->size){
        printf("error reading file\n");
        return NULL;
    }

    return tempDir;
}

/**
 * Return the index of the token if the token
 * is in the Directory
 * Returns -1 if the token is not in the directory
 */
int findInDir(DE *parent, char *token1)
{
    int numEntries = parent[0].entryCount;
    printf("Token1 %s", token1);

    for (int i = 0; i < numEntries; i++)
    {
        if(parent[i].name[0] == '\0') continue;

        if (strcmp(parent[i].name, token1) == 0)
        {
            return i;
        }
    }
    return -1;
}


DE *createFile(const char *name, DE *parent)
{

    if (!parent || !name || strlen(name) == 0)
    {
        return NULL;
    }
    printf("inside create a file, before findFreeDE\n");
    int index = findFreeDE(parent);

    if (index < 0)
    {
        printf("No space in parent\n");
        return NULL;
    }

    DE *entry = &((DE *)parent)[index];

    memset(entry, 0, sizeof(DE));
    strncpy(entry->name, name, MAX_NAME_LENGTH - 1);
    entry->name[MAX_NAME_LENGTH - 1] = '\0';

    entry->isDir = 0;
    entry->size = 0;

    time_t now = time(NULL);

    entry->creationTime = now;
    entry->modificationTime = now;
    entry->lastAccessTime = now;

    entry->mem.extentCount = 0;

    if (writeDir(parent) != 0)
    {
        printf("Failed to save parent dir\n");
        return NULL;
    }

    return entry;
}


