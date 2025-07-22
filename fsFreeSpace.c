/**************************************************************
 * Class::  CSC-415-02 Summer 2025
 * Name:: Phillip Davis, Igor Tello Jared Aung, Preet Vithani
 * Student IDs:: 923980431, 923043807, 922772159, 923575806
 * GitHub-Name:: R3plug
 * Group-Name::  Team Kentucky Kernels
 * Project:: Basic File System
 *
 * File:: fsFreeSpace.c
 *
 * Description:: Free Space manager
 *
 **************************************************************/

#include "fsFreeSpace.h"
#include "fsLow.h"
#include <stdio.h>
#include <stdlib.h>
//#include "fsPath.h"
#include "fsInit.h" 


VCB *vcb = NULL;

int loadVCB()
{
    vcb = malloc(sizeof(VCB));

    if (!vcb)
        return -1;
    
    if (LBAread(vcb, 1, 0) != 1)
    {
        free(vcb);
        vcb = NULL;
        return -1;
    }

    return 0;
}

Extent *allocateFreeBlocks(uint32_t minExtentLength, uint32_t *extentsAllocated)
{

    // Load extent table from disk
    ExtentTable *table = (ExtentTable *)malloc(EXTENT_TABLE_BLOCKS * BLOCK_SIZE);
    if (!table)
    {
        perror("Failed to allocate memory for extent table");
        return NULL;
    }

    if (LBAread(table, EXTENT_TABLE_BLOCKS, 1) != EXTENT_TABLE_BLOCKS)
    {
        perror("Failed to read extent table from disk");
        free(table);
        return NULL;
    }

    // lets now allocate spcae to store the extends we will be returning
    Extent *allocatedExtents = calloc(EXTENT_TABLE_BLOCKS, sizeof(Extent));
    if (!allocatedExtents)
    {
        perror("Failed to allocate memory for result extents");
        return NULL;
    }

    uint32_t resultIndex = 0;

    for (int i = 0; i < table->extentCount; i++)
    {
        Extent *extent = &table->extents[i];

        // we need to look for free extends that meet the size
        if (extent->used == 0 && extent->count >= minExtentLength)
        {
            /*
            // we add to the result array
            allocatedExtents[resultIndex++] = (Extent){
                .block = extent->block,
                .count = extent->count,
                .used = 1};

            // lets remove the extent from the table
            for (int j = i; j < table->extentCount - 1; j++)
            {
                table->extents[j] = table->extents[j + 1];
            }
            table->extentCount--;
            i--; // lets check the current index after shifting
            */
            // we add to the result array
            allocatedExtents[resultIndex++] = (Extent){
                .block = extent->block,
                .count = minExtentLength,
                .used = 1};

            // lets remove the extent from the table
            // set the startof the extent to the end of allcoated section
            table->extents[i].block += minExtentLength;
            break;
        }
    }

    if (resultIndex == 0)
    {
        // if no extends is allocated
        //  lets free the extend and return NULL
        *extentsAllocated = 0;
        free(allocatedExtents);
        free(table);
        return NULL;
    }

    LBAwrite(table, EXTENT_TABLE_BLOCKS, 1);
    // printf("Allocated Extents %d\n", allocatedExtents->count);
    //  lets return the allocated extents and count
    *extentsAllocated = resultIndex;
    free(table);
    return allocatedExtents;
}

int releaseBlocks(uint32_t startBlock, uint32_t blockCount)
{
    static ExtentTable *freeTable = NULL;
    if (!freeTable)
    {
        freeTable = malloc(sizeof(ExtentTable));
        if (LBAread(freeTable, EXTENT_TABLE_BLOCKS, vcb->extentTableStart) != EXTENT_TABLE_BLOCKS)
        {
            free(freeTable);
            return -1;
        }
    }

    for (int i = 0; i < EXTENT_TABLE_BLOCKS; i++)
    {
        if (freeTable->extents[i].block == startBlock &&
            freeTable->extents[i].count == blockCount)
        {
            freeTable->extents[i].used = 0;
            break;
        }
    }

    if (LBAwrite(freeTable, EXTENT_TABLE_BLOCKS, vcb->extentTableStart) != EXTENT_TABLE_BLOCKS)
    {
        perror("LBAwrite freeTable failed");
        return -1;
    }

    return 0;
}

int initFreeSpace(uint64_t numberOfBlocks, uint64_t blockSize)
{

    ExtentTable *extentTable = (ExtentTable *)calloc(1, EXTENT_TABLE_BLOCKS * blockSize);
    if (extentTable == NULL)
    {
        perror("Failed to allocate memory for extent table");
        return -1;
    }

    int index = 0;

    // Block 0 for VCB
    extentTable->extents[index++] = (Extent){.block = 0, .count = 1, .used = 1};

    // Extent table blocks
    extentTable->extents[index++] = (Extent){.block = 1, .count = EXTENT_TABLE_BLOCKS, .used = 1};

    // Root directory blocks
    extentTable->extents[index++] = (Extent){
        .block = 1 + EXTENT_TABLE_BLOCKS,
        .count = ROOT_DIRECTORY_BLOCKS,
        .used = 1};

    uint32_t freeSpaceBlocks = 1 + EXTENT_TABLE_BLOCKS + ROOT_DIRECTORY_BLOCKS;
    uint32_t remaining = numberOfBlocks - freeSpaceBlocks;
    // Rest of blocks are free space
    extentTable->extents[index++] = (Extent){.block = freeSpaceBlocks, .count = remaining, .used = 0};

    extentTable->extentCount = index;

    LBAwrite(extentTable, EXTENT_TABLE_BLOCKS, 1);

    free(extentTable);

    return 0;
}

VCB *getVCB(void) {
    VCB *vcb = malloc(sizeof(VCB));
    if (!vcb) return NULL;
    if ( LBAread(vcb, 1, 0) != 1 ) {
        free(vcb);
        return NULL;
    }
    return vcb;
}