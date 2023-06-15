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

int bmp_count = 0;
int font_count = 0;
int block8;
GSlink ghead;


void show_friendinfo_bmp(char *lcdbuf,Flink node)
{
	char path[30];
	sprintf(path,"../lib/headf/%d.bmp",node->bmp_n);
	int bmpfd  = open(path,O_RDWR);
	if(bmpfd == -1)
	{
		printf("open fail");
		return ;
	}
	int width,high;
	lseek(bmpfd,18,SEEK_SET);
	read(bmpfd,&width,4);//读取宽度
	read(bmpfd,&high,4);//读取高度
	int x_start = 2;
	int y_start = 62+bmp_count*35;
	char bmpbuf[3*width*high];
	lseek(bmpfd,54,SEEK_SET);
	read(bmpfd,bmpbuf,sizeof(bmpbuf));
	int x,y;	
	for(x = x_start;x<(x_start+width);x++)
	{
		for(y = y_start;y<(y_start+high);y++)
		{                                 
			lcdbuf[0+4*x+4*800*y] = bmpbuf[0+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[1+4*x+4*800*y] = bmpbuf[1+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[2+4*x+4*800*y] = bmpbuf[2+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[3+4*x+4*800*y] = 0;
		}
	}
	Init_Font();
	Display_characterX(50,62+bmp_count*35,node->name,0x007fb80e,1);
	Display_characterX(250,62+bmp_count*35,node->qq,0x007fb80e,1);
	UnInit_Font();
	close(bmpfd);
}

void show_groupinfo_font(char *groupname,int mebnum)
{
	char num[5];
	sprintf(num,"%d",mebnum);
	Init_Font();
	Display_characterX(402,62+font_count*35,groupname,0x007fb80e,1);
	Display_characterX(602,62+font_count*35,num,0x007fb80e,1);
	UnInit_Font();
}

//显示好友列表界面
void show_friendlist_bmp(char *lcdbuf)
{
	int bmpfd  = open("../lib/ui_templates/friendlist.bmp",O_RDWR);
	int bmpfd1 = open("../lib/ui_templates/grouplist.bmp",O_RDWR);
	if(bmpfd == -1||bmpfd1 == -1)
	{
		printf("open fail");
		return ;
	}
	int width,high;
	lseek(bmpfd,18,SEEK_SET);
	read(bmpfd,&width,4);//读取宽度
	read(bmpfd,&high,4);//读取高度
	int x_start = 0;
	int y_start = 0;
	char bmpbuf[3*width*high];
	lseek(bmpfd,54,SEEK_SET);
	read(bmpfd,bmpbuf,sizeof(bmpbuf));
	int x,y;	
	for(x = x_start;x<(x_start+width);x++)
	{
		for(y = y_start;y<(y_start+high);y++)
		{                                 
			lcdbuf[0+4*x+4*800*y] = bmpbuf[0+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[1+4*x+4*800*y] = bmpbuf[1+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[2+4*x+4*800*y] = bmpbuf[2+3*(x-x_start)+3*width*(high-1-(y-y_start))];
			lcdbuf[3+4*x+4*800*y] = 0;
		}
	}
	
	int width1,high1;
	lseek(bmpfd1,18,SEEK_SET);
	read(bmpfd1,&width1,4);//读取宽度
	read(bmpfd1,&high1,4);//读取高度
	int x_start1 = 400;
	int y_start1 = 0;
	char bmpbuf1[3*width1*high1];
	lseek(bmpfd1,54,SEEK_SET);
	read(bmpfd1,bmpbuf1,sizeof(bmpbuf1));
	int x1,y1;
	for(x1 = x_start1;x1<(x_start1+width1);x1++)
	{
		for(y1 = y_start1;y1<(y_start1+high1);y1++)
		{                                 
			lcdbuf[0+4*x1+4*800*y1] = bmpbuf1[0+3*(x1-x_start1)+3*width1*(high1-1-(y1-y_start1))];
			lcdbuf[1+4*x1+4*800*y1] = bmpbuf1[1+3*(x1-x_start1)+3*width1*(high1-1-(y1-y_start1))];
			lcdbuf[2+4*x1+4*800*y1] = bmpbuf1[2+3*(x1-x_start1)+3*width1*(high1-1-(y1-y_start1))];
			lcdbuf[3+4*x1+4*800*y1] = 0;
		}
	}
	
	Flink t = fhead;
	while(t->next!=NULL)
	{
		t = t->next;
		show_friendinfo_bmp(lcdbuf,t);
		bmp_count++;
	}
	GSlink m = ghead;
	while(m->next!=NULL)
	{
		m = m->next;
		show_groupinfo_font(m->groupname,m->mebnum);
		font_count++;
	}
	font_count = 0;
	bmp_count = 0;
	close(bmpfd);
	close(bmpfd1);
} 

//创建虚拟内存,show_friendlist_bmp()函数显示图片
void show_friendlist_emun()
{
	int lcdfd = open("/dev/fb0",O_RDWR);
	if(lcdfd == -1)
	{
		perror("open lcd fail\n");
		return ;
	}

	int x,y;
	unsigned char *lcdbuf =(unsigned char *)mmap(NULL,480*800*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcdfd,0);	
	for(x = 0;x<800;x++)
	{
		for(y = 0;y<480;y++)
		{                                 
			lcdbuf[0+4*x+4*800*y] = 255;
			lcdbuf[1+4*x+4*800*y] = 255;
			lcdbuf[2+4*x+4*800*y] = 255;
			lcdbuf[3+4*x+4*800*y] = 0;
		}
	}
	show_friendlist_bmp(lcdbuf);
}

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

//判断触屏操作
int panduan_friend()
{
	int x,y,p;
	get_xy(&x,&y,&p);
	if((x>10&&x<40&&y>30&&y<50)||(x>410&&x<440&&y>30&&y<50))
		return 5;
	if(y>15&&y<55)
	{
		if(x>60&&x<125)
			return 1;
		if(x>300&&x<365)
			return 2;
		if(x>460&&x<525)
			return 3;
		if(x>700&&x<765)
			return 4;
	}
	return 0;
}

//好友列表操作
void friendlist_oper()
{
	send(sockid,"opengroup",strlen("opengroup"),0);
	block8 = 0;
	ghead = init_group_head();
	while(!block8);
	show_friendlist_emun();
	while(1)
	{
		int ret = panduan_friend();
		if(ret == 1)
		{
			send(sockid,"onplayer",strlen("onplayer"),0);
			show_friendlist_emun();
		}
	
		else if(ret == 2)
		{
			char addqq[20];
			printf("输入你想添加的好友贪吃蛇账号:");
			scanf("%s",addqq);
			char str[30];
			sprintf(str,"addplayer %s",addqq);
			send(sockid,str,strlen(str),0);
			show_friendlist_emun();
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
			show_friendlist_emun(dir_group);
		}
	
		else if(ret == 5)
			break;
			
	}
}

void friendlist()
{
	friendlist_oper();
}