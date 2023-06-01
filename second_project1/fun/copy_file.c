#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "pthread_pool.h"

int copy_file(char *str1,char *str2)
{
	
	//打开源文件
	int old_file = open(str1,O_RDWR);
	if(old_file == -1)
	{
		printf("open %s error\n",str1);
		return -1;
	}
	//创建新文件
	int new_file = open(str2,O_RDWR|O_CREAT,0777);
	if(new_file == -1)
	{
		printf("open or create %s error\n",str2);
		return -1;
	}
	//参数修改处--修改拷贝速度
	char buf[1024*1024*5];
	int ret;
	while(1)
	{
		ret = read(old_file,buf,sizeof(buf));//读取源文件内容
		if(ret == 0)
			break;
		write(new_file,buf,ret);//将源文件内容写入复制文件中

	}
	//关闭打开的文件
	printf("%s 传输成功\n",str1);
	close(old_file);
	close(new_file);
}
