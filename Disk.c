#include <stdio.h>
#include <string.h>

#include "kernel.h"
#include "global.h"
void init_disk()
{
    disk = malloc(sizeof(Disk));
    disk_int = fopen("DISK","wb+");
	
	if(disk_int!=NULL){
        for(int i=0; i<BLOCK_SIZE; ++i)
			disk->buf_int[i]=0;
        
        for(int i = 0; i < NUM_BLOCKS; i++)
			disk_write(i, disk->buf_int);
        
        fclose(disk_int);
        disk_int = fopen("DISK","rb+");
	}
}

//写操作
void disk_write(int blockNo, byte* buffer)
{
    fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET); //重定向磁头的位置
	fwrite(buffer, sizeof(byte), BLOCK_SIZE, disk_int);
	//将缓冲区buffer中的数据写入到文件disk_int中，
	//每次写入的数据大小为sizeof(byte)字节，一共写入BLOCK_SIZE个数据项。
}

void disk_writeSuperBlock(SuperBlock* block)
{
	fseek(disk_int, 0, SEEK_SET);
    fwrite((char*)(&(block->size)), sizeof(int), 1, disk_int);
    fwrite((char*)(&(block->isize)), sizeof(int), 1, disk_int);
    fwrite((char*)(&(block->freeList)), sizeof(int), 1, disk_int);
}

void disk_writeInodeBlock(int blockNo, InodeBlock* inodeBlock)
{
	fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET);

	Inode* curInode;
	for(int i=0; i<INODE_BLOCK_SIZE; ++i){
		curInode = &(inodeBlock->node[i]);
		fwrite((char *)(&curInode->flags), sizeof(int), 1, disk_int);
		fwrite((char *)(&curInode->owner), sizeof(int), 1, disk_int);
		fwrite((char *)(&curInode->size), sizeof(int), 1, disk_int);
		
		for(int j=0; j<13; ++j)
			fwrite((char *)(&(curInode->ptr[j])), sizeof(int), 1, disk_int);

	}
}

void disk_writeIndirectBlock(int blockNo, IndirectBlock* indirectBlock)
{
    fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET);


	for(int i=0;i<IN_DIRECT_BLOCK_SIZE; ++i){
		fwrite((char *)(&(indirectBlock->ptr[i])), sizeof(int), 1,disk_int);
	}
}

//读取
void disk_read(int blockNo, byte* buffer)
{

	fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET);
	fread(buffer, sizeof(byte),BLOCK_SIZE, disk_int);
}

void disk_readInodeBlock(int blockNo, InodeBlock* inodeBlock)
{
    fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET);

	Inode* curInode;
	for(int i=0; i<INODE_BLOCK_SIZE; ++i){
		curInode = &(inodeBlock->node[i]);
		fread((char *)(&curInode->flags), sizeof(int),1,disk_int);
        fread((char *)(&curInode->owner), sizeof(int),1,disk_int);
        fread((char *)(&curInode->size), sizeof(int),1,disk_int);

		for(int j=0; j<13; ++j)
            fread((char *)(&(curInode->ptr[j])), sizeof(int),1,disk_int);
	}
}

void disk_readSuperBlock(SuperBlock* block)
{
	int temp[3];
    fseek(disk_int, 0L, SEEK_SET);
	fread((char*)temp, sizeof(int), 3, disk_int);
	block->size = temp[0];//1024
	block->isize = temp[1];//4
	block->freeList = temp[2];//5
}

void disk_readIndirectBlock(int blockNo, IndirectBlock* indirectBlock)
{
	fseek(disk_int, blockNo * BLOCK_SIZE, SEEK_SET);

	for(int i=0;i<IN_DIRECT_BLOCK_SIZE; ++i){
		fread((char *)(&(indirectBlock->ptr[i])), sizeof(int), 1, disk_int);
	}
}