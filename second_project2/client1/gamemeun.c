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
int block5 = 0;
int block6 = 0;
char head_path[30];
char head_name[20];
Flink fhead;

void show_gamemeun()
{
	printf("================游戏界面================\n");
	printf("1.聊天\n");
	printf("2.查看好友列表\n");
	printf("3.退出\n");
}

void talk_to_group()
{
	printf("请输入你聊天的群组名字:");
	char group_name[20];
	scanf("%s",group_name);
	char buf[100];
	sprintf(buf,"talkgroup %s",group_name);
	send(sockid,buf,strlen(buf),0);
	printf("开始聊天了!\n");
	char str[100];
	while(1)
	{
		bzero(str,sizeof(str));
		printf("你要发送的消息:");
		scanf("%s",str);
		send(sockid,str,strlen(str),0);	
		if(!strncmp(str,"quit",4))
			break;
		usleep(2000);		
	}
	printf("结束聊天\n");
}

//游戏界面
void gamemeun_oper(char *qq)
{
	char path[30] = {0};
	sprintf(path,"../lib/user_info/%s",qq);
	char str[50] = {0};
	sprintf(str,"ifchoose %s",path);
	send(sockid,str,strlen(str),0);
	block5 = 0;
	block6 = 0;
	while(!block5);
	
	if(block5==1)
	{
		char name[20];
		int age;
		int picture = 0;
		printf("请输入你的姓名:");
		scanf("%s",name);
		printf("请输入你的年龄:");
		scanf("%d",&age);
		printf("1、2、3、4、5、6为头像 7.退出\n");
		while(1)
		{
			int ret;
			printf("输入你的选项:");
			scanf("%d",&ret);
			if(ret == 7)
				break;
			
			else
				picture = ret;

		}
		char info[50] = {0};
		sprintf(info,"choose %s %s %d %d",qq,name,age,picture);
		send(sockid,info,strlen(info),0);
		usleep(5);
		char head[30];
		sprintf(head,"../lib/headp/%d.bmp",picture);
		strcpy(head_path,head);
		strcpy(head_name,name);
		block6 = 1;		
	}
	
	char path_friend[40];
	sprintf(path_friend,"%s/friend.txt",path);
	fhead = init_friend_head();
	FILE *file1 = fopen(path_friend,"a+");
	char tcsqq[12];
	char name[20];
	int bmp_n;
	char buf[40];
	getc(file1);
	
	if(!feof(file1))
	{
		rewind(file1);
		while(!feof(file1))
		{
			fscanf(file1,"%[^\n]\n",buf);
			sscanf(buf,"%s %s %d",tcsqq,name,&bmp_n);
			insert_friend_node(tcsqq,name,bmp_n);
		}
	}
	
	fclose(file1);
	while(!block6);
	int ret;
	show_gamemeun();
	while(1)
	{
		scanf("%d",&ret);
		if(ret == 1 )
		{
			int oper;
			printf("选择 1.私聊 2.群聊:");
			scanf("%d",&oper);
			
			if(oper == 1)
				talk_to_friend();

			
			if(oper == 2)
				talk_to_group();
			
			show_gamemeun();
		}
		
		if(ret == 2 )
		{
			friendlist(qq);
			show_gamemeun();
		}
		
		if(ret == 3 )
		{
			send(sockid,"exit",strlen("exit"),0);
			break;
		}
		
	}
	free(fhead);
}


void gamemeun(char *qq)
{
	gamemeun_oper(qq);
}