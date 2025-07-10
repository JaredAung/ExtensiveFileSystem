#include "dirLow.h"
#include "fsInit.h"
#include <math.h>

#define BLOCK_SIZE 512


int main(){

    int fsSize = 10* pow(2,20);

    int numBlocks = fsSize/BLOCK_SIZE;

    if(initFileSystem(numBlocks,BLOCK_SIZE));

    return 0;
}