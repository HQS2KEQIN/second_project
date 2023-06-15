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

int block7;
Tlink thead;

//创建聊天记录头结点
Tlink init_thead()
{
	Tlink head = (Tlink)malloc(sizeof(tlink));
	if(head == NULL)
	{
		perror("create friend_head fail\n");
		return NULL;
	}
	head->next = NULL;
	head->prev = NULL;
	return head;
}

//插入聊天信息
void insert_tnode(char *qq,char *name,char *content)
{
	Tlink t = thead;
	while(t->next!=NULL)
		t = t->next;
	Tlink node = (Tlink)malloc(sizeof(tlink));
	strcpy(node->qq,qq);
	strcpy(node->name,name);
	strcpy(node->content,content);
	node->next = NULL;
	node->prev = t;
	t->next = node;
}

//发送文件
void send_file(char *path)
{
	
	//打开源文件
	int old_file = open(path,O_RDWR);
	if(old_file == -1)
	{
		printf("open %s error\n",path);
		return;
	}
	char buf[1024];
	int ret;
	char buf3[20] = "sendok";
	while(1)
	{
		bzero(buf,sizeof(buf));
		ret = read(old_file,buf,sizeof(buf));
		if(ret == 0)
			break;
		send(sockid,buf,strlen(buf),0);
		usleep(1000);
	}
	sleep(1);
	send(sockid,buf3,strlen(buf3),0);
	close(old_file);
}


//显示可聊天的好友
void show_friend()
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

//退出后写入信息
void write_talklog(char *path)
{
	Tlink t = thead;
	FILE *file = fopen(path,"w+");
	while(t->next!=NULL)
	{
		t = t->next;
		fprintf(file,"%s %s %s\n",t->qq,t->name,t->content);
	}
	fclose(file);	
}

void talk_oper()
{
	show_friend();
	block3 = 1;
	char name[30];
	char qq[12];
	printf("请输入你想进行聊天的好友及其账号:");
	scanf("%s %s",name,qq);
	char str[100];
	sprintf(str,"ifonline %s %s",qq,name);
	send(sockid,str,strlen(str),0);
	block7 = 0;
	while(!block7);
	if(block7==-1)
		return;
	thead = init_thead();
	char path[50];
	sprintf(path,"../lib/user_info/%s/%s%s.txt",self_qq,head_name,name);
	FILE *file = fopen(path,"a+");
	fclose(file);
	bzero(str,sizeof(str));
	int ret;
	printf("================聊天界面=================\n");
	printf("1.聊天\n");
	printf("2.发送文件\n");
	printf("3.退出\n");
	while(1)
	{
		scanf("%d",&ret);
		if(ret == 1)
		{
			printf("输入你想发送的话:");
			scanf("%s",str);
			char buf[100];
			sprintf(buf,"talk %s %s",qq,str);
			send(sockid,buf,strlen(buf),0);
			insert_tnode(self_qq,head_name,str);
		}
		
		if(ret == 2)
		{
			char path[30];
			printf("请输入你想发送的文件的路径:");
			scanf("%s",path);
			char file_name[20];
			printf("再输入文件名:");
			scanf("%s",file_name);
			char snd[50]; 
			sprintf(snd,"file %s %s",file_name,qq);
			send(sockid,snd,strlen(snd),0);
			sleep(2);
			printf("开始发送文件\n");
			send_file(path);
			printf("发送完毕\n");
		}
		if(ret == 3)
		{
			block3 = 0;
			write_talklog(path);
			free(thead);
			break;
		}
	}
	
}

void talk_to_friend()
{
	talk_oper();
}