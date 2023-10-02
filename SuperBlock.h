#ifndef _SUPERBLOCK_H_
#define _SUPERBLOCK_H_


typedef struct
{
	/* data */
	// 总block数目
	int size;
	// 索引节点的数目
	int isize;
	// 未使用数据块的位置
	int freeList;
} SuperBlock;


#endif /* _SUPERBLOCK_H_ */
