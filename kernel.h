#include "global.h"
#ifndef _KERNEL_H
#define _KERNEL_H
int init();
void init_init();
void disk_write(int blockNo, byte* buffer);
void disk_writeSuperBlock(SuperBlock* block);
void disk_writeInodeBlock(int blockNo, InodeBlock* inodeBlock);
void disk_writeIndirectBlock(int blockNo, IndirectBlock* indirectBlock);
void disk_read(int blockNo, byte* buffer);
void disk_readSuperBlock(SuperBlock* block);
void disk_readInodeBlock(int blockNo, InodeBlock* inodeBlock);
void disk_readIndirectBlock(int blockNo, IndirectBlock* indirectBlock);
void fileSystem_init();
int fileSystem_formatDisk();
int fileSystem_create(int sizeOfFile);
int fileSystem_open(int inumber);
int fileSystem_seek(int fd, int offset, int whence);
int fileSystem_write(int fd, byte* buffer, int length);
void fileSystem_show();
int fileSystem_read(int inodeNo);
int fileSystem_close(int fd);
#endif