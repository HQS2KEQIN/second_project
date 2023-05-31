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
#include "needhfile.h"




int open_dir(char *src_dir,char *dest_dir,thread_pool *pool)
{
	DIR *dp = opendir(src_dir);
	if(dp == NULL)
	{
		printf("open %s error\n",src_dir);
		return -1;
	}
	struct dirent *ptr = NULL;
	while(ptr = readdir(dp))
	{
		
		if(!strcmp(ptr->d_name,".") || !strcmp(ptr->d_name,".."))
			continue;
		char *str1 = (char *)malloc(sizeof(char)*1000);
		char *str2 = (char *)malloc(sizeof(char)*1000);
		if(ptr->d_type == 4)
		{
			sprintf(str1,"%s/%s",src_dir,ptr->d_name);
			sprintf(str2,"%s/%s",dest_dir,ptr->d_name);	
			mkdir(str2,0777);
			open_dir(str1,str2,pool);
		}
		//用堆空间保存文件路径名
		else
		{
			sprintf(str1,"%s/%s",src_dir,ptr->d_name);
			sprintf(str2,"%s/%s",dest_dir,ptr->d_name);
			add_task(pool,copy_file,str1,str2);
		}

	}
	closedir(dp);
}



int main(void)
{
	//定义线程池并初始化
	thread_pool *pool = (thread_pool *)malloc(sizeof(thread_pool));
	init_pthread_pool(pool,20);
	printf("准备开始激情下载,are you ready？\n");
	for(int i = 3;i>=1;i--)
	{
		sleep(1);
		printf("%d\n",i);
	}
	sleep(1);
	open_dir("lib/dir1","lib/dir2",pool);
	sleep(1);

	destroy_pool(pool);
	printf("拷贝结束,准备撤退\n");
	sleep(1);
	return 0;
}

