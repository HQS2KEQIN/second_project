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
int talk_up;
//显示聊天信息
void show_talkfriend_font(Tlink tnode,char *qq,char *name,char *str)
{

	Init_Font();
	Display_characterX(140,5,qq,0x00fffffb,1);
	Display_characterX(6,390,str,0x00fffffb,2);
	Display_characterX(680,96,name,0x00130c0e,2);
	talk_up = 0;
	Tlink t = tnode;
	while(t!=NULL)
	{	
		if(!strcmp(t->name,name))
			Display_characterX(5,26+20*talk_up,t->content,0x007fb80e,1);
			
		else
			Display_characterX(450,26+20*talk_up,t->content,0x007fb80e,1);

		talk_up++;
		if(talk_up>17)
			break;
		t = t->next;
	}
	UnInit_Font();
}

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

//显示私聊界面
void show_talkfriend_bmp(Tlink tnode,char *lcdbuf,char *qq,char *name,char *str)
{
	int bmpfd = open("../lib/ui_templates/friendtalk.bmp",O_RDWR);
	if(bmpfd == -1)
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
	show_talkfriend_font(tnode,qq,name,str);
	close(bmpfd);
} 

//创建虚拟内存,show_talkfriend_bmp()函数显示图片
void show_talkfriend_emun(Tlink tnode,char *qq,char *name,char *str)
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

	show_talkfriend_bmp(tnode,lcdbuf,qq,name,str);
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

//判断触屏操作
int panduan_talkfriend()
{
	int x1,y1,p;
	get_xy(&x1,&y1,&p);
	if(p!=0)
	{
		if(x1>505&&x1<600&&y1>420&&y1<480)
			return 1;							//发送消息
		
		if(x1>637&&x1<746&&y1>315&&y1<375)
			return 2;							//传输文件
		
		if(x1>768&&x1<800&&y1>0&&y1<25)
			return 3;							//退出
	}
	int x2,y2;
	get_xy(&x2,&y2,&p);
	if(p == 0) 
	{
		int deta_y = y2-y1;
		if(deta_y>50)
			return 4;							//下滑,下一页
		else if(deta_y<-50)
			return 5; 							//上划,上一页
	}
	
	return 0;
}

//下一页第一个节点
Tlink find_next_node(Tlink fnode)
{
	Tlink t = fnode;
	for(int i = 0;i<18;i++)
	{
		
		if(fnode->next == NULL)
			return t;
		fnode = fnode->next;
	}
	return fnode;
}

//上一页第一个节点
Tlink find_prev_node(Tlink fnode)
{
	Tlink t = fnode;
	for(int i = 0;i<18;i++)
	{
		if(fnode->prev == NULL)
			return t;
		fnode = fnode->prev;
	}
	return fnode;
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

//找到最后一页的第一个信息节点
Tlink find_lastpage_node()
{
	Tlink t = thead->next;
	Tlink m = thead->next;
	int ret = 1;
	while(m!=NULL)
	{
		
		ret ++;
		if(ret > 18)
		{
			t = m;
			ret = 1;
		}
		m = m->next;
	}
	return t;
}

void talk_oper()
{
	show_friend();
	char name[30];
	char qq[12];
	block3 = 1;
	printf("请输入你想进行聊天的好友及其账号:");
	scanf("%s %s",name,qq);
	char str[100];
	sprintf(str,"ifonline %s %s",qq,name);
	send(sockid,str,strlen(str),0);
	block7 = 0;
	while(!block7);
	if(block7 == -1)
		return;
	thead = init_thead();
	char path[50];
	sprintf(path,"../lib/user_info/%s/%s%s.txt",self_qq,head_name,name);
	printf("path = %s\n",path);
	FILE *file = fopen(path,"a+");
	getc(file);
	bzero(str,sizeof(str));
	char tcsqq[12];
	char tcsname[20];
	char talk[100];
	if(!feof(file))
	{
		rewind(file);
		while(!feof(file))
		{

			fscanf(file,"%[^\n]\n",str);
			sscanf(str,"%s %s %s\n",tcsqq,tcsname,talk);
			insert_tnode(tcsqq,tcsname,talk);
		}
	}
	fclose(file);
	bzero(str,sizeof(str));
	Tlink fnode;
	if(thead->next == NULL)
		fnode = thead->next;
	else
		fnode = find_lastpage_node();
	
	while(1)
	{
		show_talkfriend_emun(fnode,qq,name,str);
		int ret = panduan_talkfriend();
		if(ret == 1)
		{
			printf("输入你想发送的话:");
			scanf("%s",str);
			char buf[100];
			sprintf(buf,"talk %s %s",qq,str);
			send(sockid,buf,strlen(buf),0);
			insert_tnode(self_qq,head_name,str);
			if(fnode==NULL)
				fnode = thead->next;
		}
		
		if(ret == 2)
		{
			char path[100];
			printf("请输入你想发送的文件的路径:");
			scanf("%s",path);
			char file_name[50];
			printf("再输入文件名:");
			scanf("%s",file_name);
			char snd[200]; 
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
		if(ret == 4)
			fnode = find_next_node(fnode);

		if(ret == 5)
			fnode = find_prev_node(fnode);


	}
	
}

void talk_to_friend()
{
	talk_oper();
}