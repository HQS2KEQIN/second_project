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
#include <dirent.h>
#include <sys/mman.h>
#include <linux/input.h>
#include "user_info.h"
#include "font.h"

int block4;

//注册的用户信息插入文件中
void send_register_info(char *input_acc,char *input_pn,char *input_pwd,char *input_sex)
{
	char buf[200] = {0};
	sprintf(buf,"register %s %s %s %s offline",input_acc,input_pn,input_pwd,input_sex);
	send(sockid,buf,strlen(buf),0);
}

void show_register()
{
	printf("=================注册界面===============\n");
	printf("1.输入账号\n");
	printf("2.输入电话\n");
	printf("3.输入密码\n");
	printf("4.输入性别\n");
	printf("5.确认注册\n");
	printf("6.退出注册\n");
}

//注册主页界面
void register_oper()
{
	char input_acc[12] = {0};
	char input_pn[12] = {0};
	char input_pwd[30] = {0};
	char input_sex[6] = {0};
	bool ifacc = false;
	bool ifpn  = false;
	bool ifpwd = false;
	bool ifsex = false;
	show_register();
	int ret;
	while(1)
	{
		printf("输入操作:");
		scanf("%d",&ret);
		if(ret == 1)
		{
			bzero(input_acc,sizeof(input_acc));
			printf("请输入账号:");
			scanf("%s",input_acc);
			ifacc = true;
		}
	
		else if(ret == 2)
		{
			bzero(input_pn,sizeof(input_pn));
			printf("请输入电话:");
			scanf("%s",input_pn);
			ifpn = true;
		}
	
		else if(ret == 3)
		{
			bzero(input_pwd,sizeof(input_pwd));
			printf("请输入密码:");
			scanf("%s",input_pwd);
			ifpwd = true;
		}
	
		else if(ret == 4)
		{
			bzero(input_sex,sizeof(input_sex));
			printf("请输入性别:");
			scanf("%s",input_sex);
			ifsex = true;
		}
	
		else if(ret == 5)
		{
			if(!ifacc||!ifpwd||!ifpn||!ifsex)
			{
				printf("输入的信息不全\n");
				continue;
			}
			block4 = 0;
			send_register_info(input_acc,input_pn,input_pwd,input_sex);
			while(!block4);
			if(block4 == -1)
			{
				printf("账号已被注册\n");
				bzero(input_acc,sizeof(input_acc));
				ifacc = false;
				continue;
			}
			if(block4 == -2)
			{
				printf("电话已被注册\n");
				bzero(input_pn,sizeof(input_pn));
				ifpn =  false;
				continue;
			}
			break;	
		}
		else if(ret == 6)
			break;
		
	}
}

void register_info()
{
	register_oper();
}