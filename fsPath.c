/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis Igor Tello Jared Aung Preet Vithani
* Student IDs:: , 923043807, 
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: fsPath.c
*
* Description:: 
*	
*
**************************************************************/

#include "fsPath.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dirLow.h" 
#include "mfs.h"

#define MAXSTACKSIZE 50 // Max Path Component

// We need an internal stack of the path component
static char *stack[MAXSTACKSIZE];
static char *strPath = NULL;
static int   size    = 0;

void push(const char *value) {
    if (size >= MAXSTACKSIZE) return;
    stack[size] = malloc(strlen(value) + 1);
    if (!stack[size]) return;
    strcpy(stack[size], value);
    size++;
}

void pop(void) {
    if (size <= 0) return;
    free(stack[--size]);
    stack[size] = NULL;
}

void pathCleaner(char *path) {
    // Lets reset the stack to root, if the path is absolute
    if (path[0] == '/') {
        while (size > 0) pop();
    }

    // We need to tokenize on /
    char *tok = strtok(path, "/");
    while (tok) {
        if (strcmp(tok, ".") == 0) {
            // pass
        }
        else if (strcmp(tok, "..") == 0) {
            if (size > 0) pop();
        }
        else {
            push(tok);
        }
        tok = strtok(NULL, "/");
    }
}

char *toString(void) {
    // We need to build 
    // one / per component
    // plus trailing / and null
    int needed = 1; 
    for (int i = 0; i < size; i++) {
        needed += strlen(stack[i]) + 1;
    }
    needed += 1; 

    char *buf = realloc(strPath, needed);
    if (!buf) return NULL;
    strPath = buf;

    // we need the build output
    char *p = strPath;
    *p++ = '/';
    for (int i = 0; i < size; i++) {
        size_t len = strlen(stack[i]);
        memcpy(p, stack[i], len);
        p += len;
        *p++ = '/';
    }
    *p = '\0';
    return strPath;
}

char *getCWDStr(void) {
    return toString();
}

void freeSTRCWD(void) {
    while (size > 0) pop();
    free(strPath);
    strPath = NULL;
}

void freePPI(ppInfo *ppi) {
    if (!ppi) return;
    // if this parent buffer isn’t the global cwd or root, free it
    if (ppi->parent != getCwd() && ppi->parent != getRootDir()) {
        free(ppi->parent);
    }
    free(ppi);
}

int entryIsDir(ppInfo *ppi) {
    if (!ppi || ppi->index < 0) return 0;
    printf("is directory = %d\n", ppi->parent[ppi->index].isDir);
    return (ppi->parent[ppi->index].isDir == '1');
}