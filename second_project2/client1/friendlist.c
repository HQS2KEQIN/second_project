#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <linux/input.h>
#include "user_info.h"
#include "font.h"

int block8;
GSlink ghead;

//创建好友信息头节点
Flink init_friend_head()
{
	Flink head = (Flink)malloc(sizeof(flink));
	if(head == NULL)
	{
		perror("create friend_head fail\n");
		return NULL;
	}
	head->next = NULL;
	return head;
}

//创建群组信息头结点
GSlink init_group_head()
{
	GSlink head = (GSlink)malloc(sizeof(gslink));
	if(head == NULL)
	{
		perror("create friend_head fail\n");
		return NULL;
	}
	head->next = NULL;
	return head;
}

//插入群组信息节点
void insert_group_node(char *groupname,int mebnum)
{
	GSlink t = ghead;
	while(t->next != NULL)
		t = t->next;
	GSlink node = (GSlink)malloc(sizeof(gslink));
	strcpy(node->groupname,groupname);
	node->mebnum = mebnum;
	node->next = NULL;
	t->next = node;
}

//显示好友
void show()
{
	Flink t = fhead;
	printf("你的好友有:\n");
	printf("=========================\n");
	while(t->next != NULL)
	{
		t = t->next;
		printf("%s %s\n",t->qq,t->name);
	}
	printf("=========================\n");
}

//插入好友信息头节点
void insert_friend_node(char *qq,char *name,int bmp_n)
{

	Flink t = fhead;
	while(t->next != NULL)
		t = t->next;
	
	Flink node = (Flink)malloc(sizeof(flink));
	strcpy(node->qq,qq);
	strcpy(node->name,name);
	node->bmp_n = bmp_n;
	node->next = NULL;
	t->next = node;
}

void show_friendlist()
{
	printf("==============好友列表==============\n");
	show();
	printf("1.查看在线玩家\n");
	printf("2.添加好友\n");
	printf("3.查看组\n");
	printf("4.创建群\n");
	printf("5.退出\n");
}

//好友列表操作
void friendlist_oper(char *qq)
{
	int ret;
	show_friendlist();
	
	while(1)
	{
		printf("输入你想进行的操作:");
		scanf("%d",&ret);
		if(ret == 1)
			send(sockid,"onplayer",strlen("onplayer"),0);

		else if(ret == 2)
		{
			char addqq[20];
			printf("输入你想添加的好友贪吃蛇账号:");
			scanf("%s",addqq);
			char str[30];
			sprintf(str,"addplayer %s",addqq);
			send(sockid,str,strlen(str),0);
		}
	
		else if(ret == 3)
		{
			Flink t = fhead;
			printf("你的好友有:\n");
			printf("=========================\n");
			while(t->next != NULL)
			{
				t = t->next;
				printf("%s %s\n",t->qq,t->name);
			}
			printf("=========================\n");
		}
	
		else if(ret == 4)
		{
			char groupname[20],name[20],qq[12]; 
			printf("请输入你想创建的群名:");
			scanf("%s",groupname);
			int num;
			printf("输入你想拉进群的人数:");
			scanf("%d",&num);
			char str[100];
			char dir_group[50];
			char file_group[50];	
			sprintf(dir_group,"bulidgroup %s %d",groupname,num);
			send(sockid,dir_group,strlen(dir_group),0);
			for(int i = 0;i<num;i++)
			{
				printf("输入%d人账号和名字:",i+1);
				scanf("%s %s",qq,name);
				sprintf(str,"%s %s",qq,name);
				send(sockid,str,strlen(str),0);
				usleep(3000);
			}
			send(sockid,"finish",strlen("finish"),0);
		}
	
		else if(ret == 5)
			break;
			
	}
}

void friendlist(char *qq)
{
	friendlist_oper(qq);
}