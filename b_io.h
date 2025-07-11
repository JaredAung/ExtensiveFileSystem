/**************************************************************
* Class::  CSC-415-02 Summer 2025
* Name::Phillip Davis Igor Tello Jared Aung, Preet Vithani
* Student IDs::923980431, 922772159, 923575806, 923043807
* GitHub-Name::R3plug
* Group-Name::Team Kentucky Kernels
* Project:: Basic File System
*
* File:: b_io.h
*
* Description:: Interface of basic I/O Operations
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

