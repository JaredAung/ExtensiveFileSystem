#include <stdlib.h>

#define MAX_EXTENTS 1024

typedef struct Extent{
	uint32_t block; //block location
	uint32_t count; //number of blocks at this location
	int used; // 1 if used, 0 if free
} Extent;

typedef struct ExtentTable{
	Extent extents[MAX_EXTENTS]; 
	uint32_t extentCount; //number of extents in extent table.
} ExtentTable;

typedef struct VolumeControlBlock{
	char signature[8];
	uint32_t blockSize;
	uint32_t totalBlocks;
	uint32_t extentTableStart;
	uint32_t extentTableBlocks;
	uint32_t rootDirStart;
	uint32_t rootDirBlocks;
	uint32_t freeBlockStart;

	time_t createTime;
	time_t lastMountTime;

}VCB;

Extent *allocateFreeBlocks(ExtentTable *extentTable, 
		uint32_t minExtentLength, uint32_t *extentsAllocated);