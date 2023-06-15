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
int block9 = 0;


char head_path[30];
char head_name[20];
Flink fhead;
//ͷ����ʾͼƬ
void show_choose_bmp(char *lcdbuf)
{
	int bmpfd = open("../lib/ui_templates/choose.bmp",O_RDWR);
	if(bmpfd == -1)
	{
		printf("open fail");
		return ;
	}
	
	int width,high;
	lseek(bmpfd,18,SEEK_SET);
	read(bmpfd,&width,4);//��ȡ���
	read(bmpfd,&high,4);//��ȡ�߶�
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

	close(bmpfd);
} 

//���������ڴ�,show_choose_bmp()������ʾͼƬ
void show_choose_emun()
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

	show_choose_bmp(lcdbuf);
}

//��ʾ����
void show_gamemeun_font()
{
	Init_Font();
	Display_characterX(155,0,head_name,0x00bed742,2);
	UnInit_Font();
}

//��Ϸ������ʾͼƬ
void show_gamemeun_bmp(char *lcdbuf,char *head_path)
{
	int bmpfd = open("../lib/ui_templates/gamemeun.bmp",O_RDWR);
	int bmpfd1 = open(head_path,O_RDWR);
	if(bmpfd == -1||bmpfd1 == -1)
	{
		printf("open fail");
		return ;
	}
	int width,high;
	lseek(bmpfd,18,SEEK_SET);
	read(bmpfd,&width,4);//��ȡ���
	read(bmpfd,&high,4);//��ȡ�߶�
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
	read(bmpfd1,&width1,4);//��ȡ���
	read(bmpfd1,&high1,4);//��ȡ�߶�
	int x_start1 = 0;
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
	show_gamemeun_font();
	close(bmpfd);
	close(bmpfd1);
} 

//���������ڴ�,show_gamemeun_emun()������ʾͼƬ
void show_gamemeun_emun(char *head_path)
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

	show_gamemeun_bmp(lcdbuf,head_path);
}

//�ж�ͷ��ѡ�����
int panduan_choose()
{
	int x,y,p;
	get_xy(&x,&y,&p);
	if(y>32&&y<145)
	{
		if(x>18&&x<222)
			return 1;
		
		if(x>286&&x<490)
			return 2;
		
		if(x>554&&x<758)
			return 3;
	}
	if(y>213&&y<326)
	{
		if(x>18&&x<222)
			return 4;
		
		if(x>286&&x<490)
			return 5;
		
		if(x>554&&x<758)
			return 6;
	}
	
	if(x>550&&x<730&&y>370&&y<445)
		return 7;
		
	return 0;
}

//�ж���Ϸ���津������
int	panduan_gamemeun()
{
	int x,y,p;
	get_xy(&x,&y,&p);
	if(y>430&&y<470)
	{
		if(x>20&&x<110)
			return 1;			//����
		
		if(x>130&&x<220)
			return 2;			//�鿴�����б�
		
	}
	if(y>312&&y<362)
	{
		if(x>120&&x<250)
			return 3;			//̰���߼�ģʽ
		
		if(x>262&&x<392)
			return 4;			//̰�����е�ģʽ
		
		if(x>405&&x<535)
			return 5;			//̰��������ģʽ
		
		if(x>548&&x<678)
			return 6;			//̰�����޾�ģʽ
	}
	
	if(x>690&&x<800&&y>420&&y<480)
		return 7;
		
	return 0;
}

//��ʾ��ѡ����Ƭ
void show_choose_font(int picture)
{
	char str[5] = {0};
	sprintf(str,"%d",picture);
	Init_Font();
	Display_characterX(174,402,str,0x007fb80e,2);
	UnInit_Font();
}

void talk_to_group()
{
	printf("�������������Ⱥ������:");
	char group_name[20];
	scanf("%s",group_name);
	char buf[100];
	sprintf(buf,"talkgroup %s",group_name);
	send(sockid,buf,strlen(buf),0);
	printf("��ʼ������!\n");
	block9 = 0;
	// while(!block9)
	// if(block9==-1)
		// return;
	char str[100];
	while(1)
	{
		bzero(str,sizeof(str));
		printf("��Ҫ���͵���Ϣ:");
		scanf("%s",str);
		send(sockid,str,strlen(str),0);	
		if(!strncmp(str,"quit",4))
			break;
		usleep(2000);		
	}
	printf("��������\n");
}

//��Ϸ����
void gamemeun_oper(char *qq)
{
	char path[30] = {0};
	sprintf(path,"../lib/user_info/%s",qq);
	if(access(path,F_OK))
		mkdir(path,0777);

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
		printf("�������������:");
		scanf("%s",name);
		printf("�������������:");
		scanf("%d",&age);
		show_choose_emun();
		while(1)
		{
			
			show_choose_font(picture);
			int ret = panduan_choose();
			if(!ret)
				show_choose_emun();
			
			if(ret == 7)
				break;
			
			else
			{
				picture = ret;
				show_choose_emun();
			}
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
	show_gamemeun_emun(head_path);
	while(1)
	{
		int ret = panduan_gamemeun();
		if(ret == 1 )
		{
			int oper;
			printf("ѡ�� 1.˽�� 2.Ⱥ��:");
			scanf("%d",&oper);
			
			if(oper == 1)
			{
				talk_to_friend();
				show_gamemeun_emun(head_path);
			}
			
			if(oper == 2)
			{
				talk_to_group();
				show_gamemeun_emun(head_path);
			}

		}
		if(ret == 2 )
		{
			
			friendlist(qq);
			show_gamemeun_emun(head_path);
		}
		if(ret == 3 )
		{
			printf("��ģʽ\n");
			show_gamemeun_emun(head_path);
		}
		if(ret == 4 )
		{
			printf("�е�ģʽ\n");
			show_gamemeun_emun(head_path);
		}
		if(ret == 5 )
		{
			printf("����ģʽ\n");
			show_gamemeun_emun(head_path);
		}
		if(ret == 6 )
		{
			printf("�޾�ģʽ\n");
			show_gamemeun_emun(head_path);
		}
		if(ret == 7 )
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