#include"Disk.h"
#include"misc.h"

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#define ISIZE 4
#define MAX_FILES 16

typedef struct 
{
	/* data */
	Disk* disk;
	SuperBlock superBlock;
	InodeBlock inodeBlockTable[ISIZE];
	// 文件描述表
	int fileDescTable[MAX_FILES];
	// 记录文件的偏移指针
	int seekPointer[MAX_FILES];
	int fileNum;//记录file数量

}FileSystem;


#endif /* _FILE_SYSTEM_H_ */
