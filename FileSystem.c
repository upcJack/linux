#include "kernel.h"
#include "global.h"
#include <stdio.h>
#include <string.h>

int init()
{
    int errCode;
    init_disk();
    fileSystem_init();
    errCode = fileSystem_formatDisk();
    return errCode;
}

void fileSystem_init()
{
    disk_readSuperBlock(&fileSystem.superBlock);
    for(int i = 0; i < ISIZE; i++)
	{
		disk_readInodeBlock(i+1, &fileSystem.inodeBlockTable[i]);
	}
}


int fileSystem_formatDisk()
{
	//初始化superBlock
	fileSystem.superBlock.size = NUM_BLOCKS;
	fileSystem.superBlock.isize = ISIZE;
	fileSystem.superBlock.freeList = ISIZE + 1;
	disk_writeSuperBlock(&fileSystem.superBlock);

	int freeBlockIndex = ISIZE + 1;
	int freeBlockCounter = ISIZE + 1;;

	IndirectBlock inode;
	for(int i = 0; i < POINTERS_PER_BLOCK; i++)
		inode.ptr[i] = 0;

	for(int i= 0; i < ISIZE; i++)
	{
		disk_writeIndirectBlock(i+1, &inode);
		disk_readInodeBlock(i+1, &fileSystem.inodeBlockTable[i]);
	}

	//初始化所有block
	IndirectBlock freeBlock;
	while(freeBlockIndex<NUM_BLOCKS){

		for(int i=0; i<POINTERS_PER_BLOCK ; ++i){
			freeBlock.ptr[i] = 0;
			freeBlockCounter++;
		}
		disk_writeIndirectBlock(freeBlockIndex, &freeBlock);
		freeBlockIndex++;
	}

	//初始化所有file descriptor table
	for(int i=0; i<MAX_FILES; ++i)
		fileSystem.fileDescTable[i] = 0;

	return 0;
}

int fileSystem_inumber(int fd)
{
	if(fileSystem.fileDescTable[fd] != 0)
		return fileSystem.fileDescTable[fd];
	else
		return -1;
}

int fileSystem_open(int inumber)
{


	for(int i=0; i<MAX_FILES; ++i){
		if(fileSystem.fileDescTable[i] == 0){
			fileSystem.fileDescTable[i] = inumber;
			fileSystem.seekPointer[i] = 0;

			int inodeBlockNumber = ((inumber-1)/4) + 1;//((inumber-1)/4) + 1;
	
			disk_readInodeBlock(inodeBlockNumber, &(fileSystem.inodeBlockTable[inodeBlockNumber-1]));
            disk_writeInodeBlock(inodeBlockNumber, &(fileSystem.inodeBlockTable[inodeBlockNumber-1]));

			return i;
		}
	}

	return -1;
}

int fileSystem_close(int fd)
{
	int inumber = fileSystem.fileDescTable[fd];

	int inodeBlockNumber = ((inumber-1)/4)+1;

	disk_writeInodeBlock(inodeBlockNumber, &(fileSystem.inodeBlockTable[inodeBlockNumber-1]));

	fileSystem.fileDescTable[fd] = 0;
	return 0;
}

int fileSystem_seek(int fd, int offset, int whence)
{

	switch(whence){
		case SEEK_SET:
			fileSystem.seekPointer[fd] = offset;
			return fileSystem.seekPointer[fd];
		case SEEK_CUR:
			fileSystem.seekPointer[fd] += offset;
			return fileSystem.seekPointer[fd];
		case SEEK_END:
			// TODO
			break;
	}
	return -1;
}

int fileSystem_write(int fd, byte* buffer, int length)
{
	int* fileDescTable = fileSystem.fileDescTable;
	InodeBlock* inodeBlockTable = fileSystem.inodeBlockTable;
	int* seekPointer = fileSystem.seekPointer;

	int inumber = fileDescTable[fd];
	int inodeBlockNumber = ((inumber-1)/4);
	Inode inode = inodeBlockTable[inodeBlockNumber].node[(inumber-1)%4];

	int seekPtr = seekPointer[fd];

	if(seekPtr<0)
		return -1;

	int origLength = length;

	int blockNo;
	IndirectBlock indirectBlock;
	char buf_filesys[BLOCK_SIZE];
	int pos = 0;
	int offset;

	while(length > 0){
		if(seekPtr<10*BLOCK_SIZE){
			blockNo = inode.ptr[seekPtr/BLOCK_SIZE];
			if(blockNo <= 0){
				blockNo = fileSystem_allocate();
				inode.ptr[seekPtr/BLOCK_SIZE] = blockNo;
				inodeBlockTable[inodeBlockNumber].node[(inumber -1)%4] = inode;
				disk_writeInodeBlock(inodeBlockNumber+1, &inodeBlockTable[inodeBlockNumber]);

			}
		}
		// 0-9个直接块
		seekPtr -= 10*BLOCK_SIZE;
		// 一级索引
		if(seekPtr>0 && seekPtr<64*BLOCK_SIZE){
			if(inode.ptr[10] == 0){
				inode.ptr[10] = fileSystem_allocate();
			}
			disk_readIndirectBlock(inode.ptr[10], &indirectBlock);
			blockNo = indirectBlock.ptr[seekPtr/BLOCK_SIZE];
			if(blockNo <= 0){
				blockNo = fileSystem_allocate();
				indirectBlock.ptr[seekPtr/BLOCK_SIZE] = blockNo;
				disk_writeIndirectBlock(inode.ptr[10], &indirectBlock);
			}

		}
		seekPtr -= 64*BLOCK_SIZE;

		// 二级索引
		if(seekPtr>0 && seekPtr<64*64*BLOCK_SIZE){
			if(inode.ptr[11] == 0){
				inode.ptr[11] = fileSystem_allocate();
			}
			disk_readIndirectBlock(inode.ptr[11], &indirectBlock);
			if(indirectBlock.ptr[seekPtr/(64*BLOCK_SIZE)] == 0){
				indirectBlock.ptr[seekPtr/(64*BLOCK_SIZE)] = fileSystem_allocate();
				disk_writeIndirectBlock(inode.ptr[11], &indirectBlock);
			}
			disk_readIndirectBlock(indirectBlock.ptr[seekPtr/(64*BLOCK_SIZE)], &indirectBlock);
			blockNo = indirectBlock.ptr[(seekPtr%(64*BLOCK_SIZE))/BLOCK_SIZE];
			if(blockNo <= 0){
				blockNo = fileSystem_allocate();
				indirectBlock.ptr[(seekPtr%(64*BLOCK_SIZE))/BLOCK_SIZE] = blockNo;
				disk_writeIndirectBlock(indirectBlock.ptr[seekPtr/(64*BLOCK_SIZE)], &indirectBlock);
			}
		}
		seekPtr -= 64*64*BLOCK_SIZE;

		// 三级索引
		if(seekPtr>0){
			if(inode.ptr[12] == 0){
				inode.ptr[12] = fileSystem_allocate();
			}
			disk_readIndirectBlock(inode.ptr[12], &indirectBlock);
			if(indirectBlock.ptr[(seekPtr/(BLOCK_SIZE*64*64))] == 0){
				indirectBlock.ptr[(seekPtr/(BLOCK_SIZE*64*64))] = fileSystem_allocate();
				disk_writeIndirectBlock(inode.ptr[12], &indirectBlock);
			}
			disk_readIndirectBlock(indirectBlock.ptr[(seekPtr/(BLOCK_SIZE*64*64))], &indirectBlock);
			if(indirectBlock.ptr[(seekPtr%(BLOCK_SIZE*64*64))/(64*BLOCK_SIZE)] == 0){
				indirectBlock.ptr[(seekPtr%(BLOCK_SIZE*64*64))/(64*BLOCK_SIZE)] = fileSystem_allocate();
				disk_writeIndirectBlock(indirectBlock.ptr[(seekPtr/(BLOCK_SIZE*64*64))], &indirectBlock);
			}
			disk_readIndirectBlock(indirectBlock.ptr[(seekPtr%(BLOCK_SIZE*64*64))/(64*BLOCK_SIZE)], &indirectBlock);
			blockNo = indirectBlock.ptr[(seekPtr%(64*BLOCK_SIZE))/BLOCK_SIZE];
			if(blockNo <= 0){
				blockNo = fileSystem_allocate();
				indirectBlock.ptr[(seekPtr%(64*BLOCK_SIZE))/BLOCK_SIZE] = blockNo;
				disk_writeIndirectBlock(indirectBlock.ptr[(seekPtr%(BLOCK_SIZE*64*64))/(64*BLOCK_SIZE)], &indirectBlock);
			}
		}

		disk_read(blockNo, buf_filesys);

		offset = seekPointer[fd]%BLOCK_SIZE;
		if(length<BLOCK_SIZE){
			memcpy((char *)&buf_filesys[offset], (char *)&buffer[pos], length);
			// printf("%s\n", buf_filesys);
			seekPointer[fd] += length;
		}
		else{
			memcpy(&buf_filesys[offset], &buffer[pos], BLOCK_SIZE - offset);
			seekPointer[fd] += BLOCK_SIZE - offset;
		}
		printf("在第 %d 个block中写入了 \"%s\" \n",blockNo, &buf_filesys[offset]);

		disk_write(blockNo, buf_filesys);

		pos += BLOCK_SIZE - offset;
		seekPtr = seekPointer[fd];
		length -= BLOCK_SIZE - offset;
	}

	inodeBlockTable[inodeBlockNumber].node[(inumber-1)%4] = inode;
	disk_writeInodeBlock(inodeBlockNumber+1, &inodeBlockTable[inodeBlockNumber]);
	byte buf_test[256];
	disk_read(blockNo, buf_test);

	return origLength;
}

//找到空白的Block，记录位置
int fileSystem_allocate()
{

	int freeList = fileSystem.superBlock.freeList;
	IndirectBlock freeListBlock;
	disk_readIndirectBlock(freeList, &freeListBlock);


	int freeBlock = freeList;
	int counter = 0;
	BOOL wholeBlockZero = false;

    while(!wholeBlockZero){
			//当该字节不为零，说明本行（即本BLock）有数据，直接寻找下一行
      if(freeListBlock.ptr[counter] != 0){

			freeListBlock.ptr[counter] = 0;
			disk_readIndirectBlock(freeList, &freeListBlock);
			freeBlock = ++freeList;
            counter=0;
            continue;
		}

        counter+=1;
				//当counter==64时，说明本行已经遍历完，本行全部是零，BLock为空
		if(counter == POINTERS_PER_BLOCK){
			freeList++;
			wholeBlockZero=true;
			disk_readIndirectBlock(freeList, &freeListBlock);
		}

    }

	char buffer[BLOCK_SIZE] = {0};
	disk_write(freeBlock, buffer);

    //修改superBlock
    fileSystem.superBlock.size = NUM_BLOCKS;
	fileSystem.superBlock.isize = ISIZE;
	fileSystem.superBlock.freeList = freeList;
    disk_writeSuperBlock(&fileSystem.superBlock);

	return freeBlock;
}

int fileSystem_free(int blockNo)
{
	int freeBlockIndex = (blockNo - (1+ISIZE+NUM_BLOCKS/POINTERS_PER_BLOCK))/POINTERS_PER_BLOCK;
	int freeList = fileSystem.superBlock.freeList;
	IndirectBlock freeListBlock;
	disk_readIndirectBlock(freeList+freeBlockIndex, &freeListBlock);

	freeListBlock.ptr[(blockNo - (1+ISIZE+NUM_BLOCKS/POINTERS_PER_BLOCK))%POINTERS_PER_BLOCK] = blockNo;
	disk_writeIndirectBlock(freeList+freeBlockIndex, &freeListBlock);

	return 0;
}

//创建fileSystem，注意创建文件是要修改filenum
int fileSystem_create(int sizeOfFile)
{
    fileSystem.fileNum++;

	int inode;
	InodeBlock inodeBlock;

	for(int i=0; i<ISIZE; ++i){
		inodeBlock = fileSystem.inodeBlockTable[i];
		for(int j=0; j<4; ++j)
			if(inodeBlock.node[j].flags ==0){//修改inodeBLockTable

				inodeBlock.node[j].flags = 1;
				inodeBlock.node[j].owner = 1;
				inodeBlock.node[j].size = sizeOfFile;

				inode = i*4+j+1;

				fileSystem.inodeBlockTable[i] = inodeBlock;
				disk_writeInodeBlock(i+1, &fileSystem.inodeBlockTable[i]);

				return inode;
			}
	}
	return -1;
}

void fileSystem_show()
{
	printf("\n--- 文件展示开始 ---\n\n");
	IndirectBlock test;

	for(int i = 0; i < 30; i++){
		disk_readIndirectBlock(i, &test);
		for(int j = 0; j < POINTERS_PER_BLOCK; j++)
			printf("%d ",test.ptr[j]);
		printf("\n");
	}
    printf("\n--- 文件展示结束 ---\n\n");

}

int fileSystem_read(int fd)
{
	int* fileDescTable = fileSystem.fileDescTable;
	InodeBlock* inodeBlockTable = fileSystem.inodeBlockTable;
	int* seekPointer = fileSystem.seekPointer;

	int inumber = fileDescTable[fd];
	int inodeBlockNumber = ((inumber-1)/4);
	Inode inode = inodeBlockTable[inodeBlockNumber].node[(inumber-1)%4];

	int seekPtr = seekPointer[fd];

	if(seekPtr<0)
		return -1;
	int i,j;
	IndirectBlock indirectBlock;
	for (i = 0; i<13; i++){
		// 直接块
		if(i<10){
			if (inode.ptr[i]==0){
				break;
			}
			byte buf_test[256];
			disk_read(inode.ptr[i], buf_test);
			printf("%s",buf_test);
		}else if (i==10){
			disk_readIndirectBlock(inode.ptr[10], &indirectBlock);
			for (j=0;j<IN_DIRECT_BLOCK_SIZE;j++){
				byte buf_test[256];
				disk_read(indirectBlock.ptr[j], buf_test);
				printf("%s",buf_test);
			}
		}else if(i==11){
			// TODO
		}else if(i==12){
			// TODO
		}
	}
	printf("\n");

	return 0;
}