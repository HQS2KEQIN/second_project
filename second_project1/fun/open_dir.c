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


TL head_init()
{
	TL head = (TL)malloc(sizeof(tl));
	if(head == NULL)
	{
		printf("创建失败\n");
		return NULL;
	}
	head->next = NULL;
	return head;
}

void add_node(TL head,char *str1,char *str2)
{
	TL t = head;
	while(t->next!=NULL)
		t = t->next;
	
	TL node = (TL)malloc(sizeof(tl));
	if(node == NULL)
	{
		printf("创建失败\n");
		return ;
	}
	strcpy(node->str1,str1);
	strcpy(node->str2,str2);
	node->next = NULL;
	t->next = node;
}

int open_dir(TL head,char *src_dir,char *dest_dir)
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
		//属于堆空间的指针只能指向堆空间里面的变量
		char *str1 = (char *)malloc(sizeof(char)*1000);
		char *str2 = (char *)malloc(sizeof(char)*1000);
		if(ptr->d_type == 4)
		{
			sprintf(str1,"%s/%s",src_dir,ptr->d_name);
			sprintf(str2,"%s/%s",dest_dir,ptr->d_name);	
			mkdir(str2,0777);
			//递归打开目录，并在另一个路径下创建
			open_dir(head,str1,str2);
		}
		//用堆空间保存文件路径名
		else
		{
			sprintf(str1,"%s/%s",src_dir,ptr->d_name);
			sprintf(str2,"%s/%s",dest_dir,ptr->d_name);
			add_node(head,str1,str2);
		}		
	}
	closedir(dp);
}
