#include <stdio.h>
#ifndef _DISK_H_
#define _DISK_H_

#include"Inode.h"
#include"SuperBlock.h"
#include"misc.h"

#define BLOCK_SIZE 256
#define NUM_BLOCKS 1024
// 每个块的指针
#define POINTERS_PER_BLOCK ((BLOCK_SIZE)/(sizeof(int)))


typedef struct 
{
	/* data */
	byte buf_int[BLOCK_SIZE];
	FILE* disk_int;
} Disk;



#endif	/* _DISK_H_ */
