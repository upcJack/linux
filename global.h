#include "Disk.h"
#include "FileSystem.h"
#include "Inode.h"
#include "misc.h"
#ifndef __ALL_H
#define __ALL_H

typedef enum {false,true} BOOL;
FILE *disk_int;
Disk *disk;
FileSystem fileSystem;

#endif // __ALL_H 
