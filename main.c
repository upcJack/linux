#include "kernel.h"
#include "global.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

int get_len(char str[]) // 得到字符串真实长度
{
	char *p = str;
	int count = 0;
	while (*p++ != '\0')
	{
		count++;
	}
	return count;
}

void operate(){
	char opration[30];
	strcpy(opration,"ls");
	while(strcmp(opration, "exit")){ //没有退出的话，一直循环
		printf(">>$");
		scanf("%s", opration);
		if(!strcmp(opration, "format"))
		{
            fileSystem_formatDisk();
            printf("格式化成功\n");

		}
		else if(!strcmp(opration, "write"))
		{
			printf("请输入您想要写入的字符串，长度小于2000\n");
			byte buffer[2000];
			scanf("%s",buffer);
			int file1 = fileSystem_create(get_len(buffer) + 1);
			int fileid1 = fileSystem_open(file1);
			printf("请输入您想写入文件的偏移\n");
			int offset;
			scanf("%d",&offset);
			fileSystem_seek(fileid1, offset, SEEK_SET);   
			fileSystem_write(fileid1, buffer, get_len(buffer) + 1); 
			fileSystem_show();
			fileSystem_close(fileid1);

		}
		else if(!strcmp(opration, "read"))
		{
			int file1;
			printf("请输入想要读取第几个文件\n");
			scanf("%d", &file1);
			int fileid1 = fileSystem_open(file1);
			fileSystem_seek(fileid1, 0, SEEK_SET); 
			fileSystem_read(fileid1);
			fileSystem_close(fileid1);
		}
		else if(!strcmp(opration, "fdisk"))
		{
			printf("fdisk\n");
			printf("文件数目：%d\n",fileSystem.fileNum);
			printf("总block数目：%d\n",fileSystem.superBlock.size);
			printf("索引节点开始位置：%d\n",fileSystem.superBlock.isize);
			printf("未使用数据块的位置：%d\n",fileSystem.superBlock.freeList);
		}
		else if(!strcmp(opration, "show"))
		{
            fileSystem_show();
		}
		else if(!strcmp(opration, "help"))
		{
			printf("文件系统包含命令：\n");
			printf("format       格式化\n");
			printf("show         显示磁盘结构信息\n");
			printf("read         读出文件内容\n");
			printf("write        写入文件\n");
			printf("exit         退出文件系统\n");
			printf("fdisk        打印filesystem的信息\n");
			printf("help         帮助\n");
			printf("\n");

		} 
		else if(!strcmp(opration, "exit"))
		{
			continue;
		}
		else 
		{
			printf("no such operation!\nget help by input h\n");
		}
	}
	
}

int main(){
    // 初始化
    int errorCode;
    errorCode = init();
    if(errorCode == 1)
	{
		printf("Filesystem formatting failed");
	}
    fileSystem_show();
    byte buffer[] = "write test"; // 写入文件测试
	int file1 = fileSystem_create(sizeof(buffer));
	int fileid1 = fileSystem_open(file1);
    fileSystem_seek(fileid1, 0, SEEK_CUR);   
    fileSystem_write(fileid1, buffer, sizeof(buffer)); 
    fileSystem_show();
    fileSystem_close(fileid1);  
	printf("文件系统包含命令：\n");
	printf("format       格式化\n");
	printf("show         显示磁盘结构信息\n");
	printf("read         读出文件内容\n");
	printf("write        写入文件\n");
	printf("exit         退出文件系统\n");
	printf("fdisk        打印filesystem的信息\n");
	printf("help         帮助\n");
	printf("\n");
    operate();
    return 0;
}
