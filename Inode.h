#ifndef __INODE_H_
#define __INODE_H_

#define DISK_BLOCK_SIZE 256
#define INODE_SIZE 64
#define INODE_BLOCK_SIZE ((DISK_BLOCK_SIZE) / (INODE_SIZE))
#define IN_DIRECT_BLOCK_SIZE ((DISK_BLOCK_SIZE)/(sizeof(int)))

typedef struct 
{
	/* data */
	// 是否使用
	int flags;
	// 用户
	int owner;
	// 大小
	int size;
	// 0-9为直接地址 10 一次间接 11 二次间接 12 三次间接
	int ptr[13];

} Inode;

typedef struct 
{
	/* data */
	Inode node[INODE_BLOCK_SIZE];

} InodeBlock;

typedef struct 
{
	/* data */
	int ptr[IN_DIRECT_BLOCK_SIZE];
} IndirectBlock;



#endif /* _INODE_H_ */
