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
//显示注册界面
void show_register_bmp(char *lcdbuf)
{
	int bmpfd = open("../lib/ui_templates/register.bmp",O_RDWR);
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

	close(bmpfd);
} 

//创建虚拟内存,调用show_register_bmp()函数显示图片
void show_register_emun()
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

	show_register_bmp(lcdbuf);
}

//判断注册操作
int panduan_register()
{
	int x,y,p;
	get_xy(&x,&y,&p);
	if(x>207&&x<600)
	{
		if(y>158&&y<198)
			return 1;					//输入账号
		else if(y>226&&y<266)
			return 2;					//输入电话			

	}
	if(x>207&&x<536&&y>295&&y<335)
		return 3;						//输入密码
	
	if(x>536&&x<600&&y>295&&y<335)
		return 4;						//输入性别
	
	if(x>203&&x<601&&y>405&&y<450)
		return 5;						//注册按钮
	
	if(x>730&&x<790&&y>0&&y<55)			
		return 6;						//退出注册
	
	return 0;							//空白部分

}

//打印字
void show_font_register(char *str_acc,char *str_pn,char *str_psw,char *str_sex)
{
	Init_Font();
	Display_characterX(210,160,str_acc,0x007fb80e,2);
	Display_characterX(210,228,str_pn,0x007fb80e,2);
	Display_characterX(210,296,str_psw,0x007fb80e,2);
	Display_characterX(540,296,str_sex,0x007fb80e,2);
	UnInit_Font();
}

//注册的用户信息插入文件中
void send_register_info(char *input_acc,char *input_pn,char *input_pwd,char *input_sex)
{
	char buf[200] = {0};
	sprintf(buf,"register %s %s %s %s offline",input_acc,input_pn,input_pwd,input_sex);
	send(sockid,buf,strlen(buf),0);
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
	show_register_emun();
	while(1)
	{
		show_font_register(input_acc,input_pn,input_pwd,input_sex);
		int ret = panduan_register();
		if(ret == 1)
		{
			bzero(input_acc,sizeof(input_acc));
			printf("请输入账号:");
			scanf("%s",input_acc);
			ifacc = true;
			show_register_emun();
		}
	
		else if(ret == 2)
		{
			bzero(input_pn,sizeof(input_pn));
			printf("请输入电话:");
			scanf("%s",input_pn);
			ifpn = true;
			show_register_emun();
		}
	
		else if(ret == 3)
		{
			bzero(input_pwd,sizeof(input_pwd));
			printf("请输入密码:");
			scanf("%s",input_pwd);
			ifpwd = true;
			show_register_emun();
		}
	
		else if(ret == 4)
		{
			bzero(input_sex,sizeof(input_sex));
			printf("请输入性别:");
			scanf("%s",input_sex);
			ifsex = true;
			show_register_emun();
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