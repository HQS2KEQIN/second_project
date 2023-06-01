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

int main(void)
{
	//定义线程池并初始化
	thread_pool *pool = (thread_pool *)malloc(sizeof(thread_pool));
	init_pthread_pool(pool,20);
	char str1[100] = {0};
	char str2[100] = {0};
	printf("请输入要复制文件的路径:");
	scanf("%s",str1);
	printf("请输入要拷贝到的路径");
	scanf("%s",str2);
	printf("准备开始激情下载,are you ready？\n");
	for(int i = 3;i>=1;i--)
	{
		sleep(1);
		printf("%d\n",i);
	}
	
	sleep(1);
	TL head = head_init();
	if(open_dir(head,str1,str2) == -1)
	{
		destroy_pool(pool);
		printf("未找到对应路径\n");
	}

	else
	{
		while(head->next!=NULL)
		{
			add_task(pool,copy_file,head->str1,head->str2);
			head = head->next;
		}
		sleep(1);
		destroy_pool(pool);
		printf("拷贝结束,准备撤退\n");
		sleep(1);
	}
	free(head);
	return 0;
}


