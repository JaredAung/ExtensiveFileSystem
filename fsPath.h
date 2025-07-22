/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis Igor Tello Jared Aung Preet Vithani
* Student IDs::
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: fsPath.h
*
* Description:: 
*	
*
**************************************************************/

#ifndef FSPATH_H
#define FSPATH_H
 
#include "dirLow.h" 
#include <stddef.h>

// Additional helper functions for the below functions
void freePPI(ppInfo *ppi);
int  entryIsDir(ppInfo *ppi);


// We need to push the path component to the stack
void push(const char *value);

// We need to Pop the top component off the stack
void pop(void);

// we need to normalize the path string 
void pathCleaner(char *path);

// We need to build a string from the stack
char *toString(void);

// We need to get the CWD string
char *getCWDStr(void);

// Lets free our stack memory and the CWD buffer
void freeSTRCWD(void);


#endif // FSPATH_H
