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

#define IP "192.168.13.52"           //主机的ip地址
#define PORT 50003				     //主机的端口号


int sockid;
int block1;
int block2;
int block3 = 0;

int input_count = 3;
char self_qq[12];

int open_lcd()
{
	int lcdfd = open("/dev/input/event0",O_RDWR);
	if(lcdfd == -1)
	{
		perror("open lcd fail\n");
		return -1;
	}
	return lcdfd;
}

//功能:打印主页图片
void show_login_bmp(char *lcdbuf)
{
	int bmpfd = open("../lib/ui_templates/login.bmp",O_RDWR);
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

//创建虚拟地址，并对页面进行清屏处理
void show_login_emun()
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
	show_login_bmp(lcdbuf);
}

//获取x、y坐标位置
void get_xy(int *x,int *y,int *p)
{
	int lcdt =  open_lcd();
	struct input_event en;
	bool x_if = false;
	bool y_if = false;
	bool p_if = false;
	while(1)
	{
		bzero(&en,sizeof(en));
		read(lcdt,&en,sizeof(en));
		if(en.type == EV_ABS)
		{
			if(en.code == ABS_X)
			{
				x_if = true;
				*x = en.value*800/1024;
			}
			if(en.code == ABS_Y)
			{
				y_if = true;
				*y = en.value*480/600;
			}
		}
		if(en.type == EV_KEY)
		{
			if(en.code == BTN_TOUCH)
			{
				*p = en.value;
				p_if = true;
			}
		}
		if(x_if&&y_if&&p_if)
			break;
	}
}

//判断坐标操作
int panduan()
{
	int x,y,p;
	get_xy(&x,&y,&p);
	if(x>244&&x<550)
	{
		if(y>281&&y<324)
			return 1;			//输入账号
		else if(y>325&&y<366)
			return 2;			//输入密码
	}
	else if(x>615&&x<735)
	{
		if(y>281&&y<324)
			return 3;			//注册账号
		else if(y>325&&y<366)
			return 4;			//找回密码			
	}
	if(x>245&&x<610&&y>417&&y<460)	//登陆
		return 5;
	
	if(x>760&&x<790&&y>0&&y<25)
		return 6;					//退出
	
	return 0;						//其它空白部分
}

//清屏
void clear_lcd()
{
	Init_Font();
	Clean_Area(0,0,800,480,0x00000000);
	UnInit_Font();
}

//显示文字
void show_login_title(char *qq,char *passwd)
{
	Init_Font();
	Display_characterX(248,287,qq,0x00905a3d,3);
	Display_characterX(248,327,passwd,0x00905a3d,3);
	UnInit_Font();
}

//接收文件
void recv_file(char *path)
{
	int new_file = open(path,O_RDWR|O_CREAT,0777);
	if(new_file == -1)
	{
		printf("open or create %s error\n",path);
		return ;
	}
	char buf[1024] = {0};
	while(1)
	{
		bzero(buf,sizeof(buf));
		recv(sockid,buf,sizeof(buf),0);
		if(!strcmp(buf,"sendok"))
			break;

		write(new_file,buf,strlen(buf));//将源文件内容写入复制文件中
		
	}
	close(new_file);
	
	
}

//接收服务器发送信息的线程
void *recv_data(void *arg)
{	
	char str1[20];
	char str2[20];
	char str3[20];
	char buf[100];
	int ret = 1;
	while(1)
	{
		bzero(buf,sizeof(buf));
		bzero(str1,sizeof(str1));
		bzero(str2,sizeof(str2));
		bzero(str3,sizeof(str3));
		recv(sockid,buf,sizeof(buf),0);
		//输入的信息错误
		if(!strncmp(buf,"false",5))
		{
			sscanf(buf,"%s %s",str1,str2);
			if(!strncmp(str2,"qq",2))
				printf("贪吃蛇账号不存在\n");
			if(!strncmp(str2,"passwd",6))
				printf("密码错误\n");
			block2 = -1;
			continue;
		}
		//信息正确
		if(!strncmp(buf,"true",4))
			block2 = 1;		
		
		//允许注册
		if(!strncmp(buf,"accessr",7))
			block4 = 1;
		
		//禁止注册,因为电话重复
		if(!strncmp(buf,"banrgpn",7))
			block4 = -2;
		
		//禁止注册,因为账号重复
		if(!strncmp(buf,"banrgacc",8))
			block4 = -1;
		
		//第一次注册账号登陆，录入信息
		if(!strncmp(buf,"noexist",7))
			block5 = 1;

		//无需录入
		if(!strncmp(buf,"yesexist",8))
		{
			block5 = -1;
			char str[10];
			char path[40];
			char name[20];
			sscanf(buf,"%s %s %s",str,path,name);
			strcpy(head_path,path);
			strcpy(head_name,name);
			block6 = 1;
		}

		//接收服务器发送的在线玩家信息
		if(!strncmp(buf,"check",5))
		{
			if(ret)
			{
				printf("在线玩家列表\n");
				printf("========================\n");
				printf("tcsqq\t\tname\n");
				ret = 0;
			}
			sscanf(buf,"%s %s %s",str1,str2,str3);
			printf("%s\t%s\n",str2,str3);
		}
		
		//服务器发送所有信息完毕
		if(!strncmp(buf,"listok",6))
		{
			printf("========================\n");
			ret = 1;
		}
		
		if(!strncmp(buf,"nofindfri",9))
			printf("没有此玩家\n");
		
		//添加好友
		if(!strncmp(buf,"addplayer",9))
		{
			char name[20];
			char qq[20];
			int pic;
			sscanf(buf,"%s %s %s %d",str1,name,qq,&pic);
			char path[50];
			sprintf(path,"../lib/user_info/%s/friend.txt",self_qq);
			FILE *file = fopen(path,"a+");
			fprintf(file,"\n%s %s %d\n",qq,name,pic);
			fclose(file);
			insert_friend_node(qq,name,pic);
		}
	
		//好友不在线
		if(!strncmp(buf,"noonline",8))
		{
			printf("好友不在线\n");
			block7 = -1;
		}
	
		//好友在线
		if(!strncmp(buf,"yesonline",8))
			block7 = 1;
		
		//发送的聊天信息
		if(!strncmp(buf,"talk",4))
		{
			char qq[12],name[20],content[100];
			sscanf(buf,"%s %s %s %s",str1,qq,name,content);
			printf("账号为%s的%s发给你信息:%s\n",qq,name,content);
			char path[50];
			sprintf(path,"../lib/user_info/%s/%s%s.txt",self_qq,head_name,name);
			FILE *file = fopen(path,"a+");
			if(block3)
				insert_tnode(qq,name,content);
			fprintf(file,"%s %s %s\n",qq,name,content);
			fclose(file);	
		}
	
		//接收文件
		if(!strncmp(buf,"file",4))
		{
			char qq[12],name[20],file_name[50];
			sscanf(buf,"%s %s %s %s",str1,qq,name,file_name);
			printf("接收到账号为%s的%s发送的文件:%s\n",qq,name,file_name);
			char path[100];
			char dir_path[100];
			sprintf(dir_path,"../lib/user_info/%s/file",self_qq);
			if(access(dir_path,F_OK))
				mkdir(dir_path,0777);
			sprintf(path,"%s/%s",dir_path,file_name);
			printf("开始接收文件\n");
			recv_file(path);	
			printf("接收完成\n");
		}
	
		//接收群组信息
		if(!strncmp(buf,"yesgroup",8))
		{	
			char groupname[20];
			int mebnum;
			while(1)
			{
				bzero(buf,sizeof(buf));
				recv(sockid,buf,sizeof(buf),0);
				if(!strncmp(buf,"finish",6))
					break;
				sscanf(buf,"%s %d",groupname,&mebnum);
				insert_group_node(groupname,mebnum);
			}
			block8 = 1;
		}
		
		//接收群消息
		if(!strncmp(buf,"gtalk",5))
		{
			char groupname[20],qq[12],name[20],talk[100];
			sscanf(buf,"%s %s %s %s %s",str1,groupname,qq,name,talk);
			printf("接收到群名为%s、账号%s、用户%s发送的群消息:%s\n",groupname,qq,name,talk);
		}
			
		if(!strncmp(buf,"nogroup",7))
			block8 = -1;
	}
	
}

//主界面
void index_oper()
{
	sockid = socket(AF_INET,SOCK_STREAM,0);
	
	if(sockid == -1)
	{
		perror("套接字创建失败\n");
		return ;
	}
	struct sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	inet_pton(AF_INET,IP,&client.sin_addr);
	if(connect(sockid,(struct sockaddr *)&client,sizeof(client))== -1)
	{
		perror("连接失败\n");
		return ;
	}
	pthread_t tid;
	pthread_create(&tid,NULL,recv_data,NULL);
	char qq[12] = {0};
	char passwd[30] = {0};
	char buf[100];
	bool ifqq = false;
	bool ifpasswd = false;
	show_login_emun();
	while(1)
	{
		show_login_title(qq,passwd);
		int ret = panduan();
		if(ret == 1)
		{
			printf("请输入贪吃蛇账号:");
			scanf("%s",qq);
			ifqq = true;
			show_login_emun();
		}
		
		else if(ret == 2)
		{
			printf("请输入密码:");
			scanf("%s",passwd);
			ifpasswd = true;
			show_login_emun();
		}
		
		else if(ret == 3)
		{
			register_info();	
			show_login_emun();
		}
		
		else if(ret == 4)
		{
			printf("找回密码\n");
			show_login_emun();
		}
		
		else if(ret == 5)
		{			
			if(!ifqq||!ifpasswd)
			{
				printf("信息输入不全\n");
				show_login_emun();
				continue;
			}
			block2 = 0;
			bzero(buf,sizeof(buf));
			sprintf(buf,"login %s %s",qq,passwd);
			send(sockid,buf,strlen(buf),0);
			while(!block2);
			if(block2 == -1)
			{		
				input_count--;
				if(input_count)
					printf("还有%d次输入机会\n",input_count);
				else
					printf("错误次数超过三次，不许登陆\n");
				bzero(qq,sizeof(qq));
				bzero(passwd,sizeof(passwd));
				ifqq = false;
				ifpasswd = false;
				continue;
			}
			printf("登陆成功\n");
			strcpy(self_qq,qq);
			gamemeun(qq);
			bzero(qq,sizeof(qq));
			bzero(passwd,sizeof(passwd));
			show_login_emun();
		}
		
		else if(ret == 6)
		{
			block3 = 0;
			send(sockid,"quit",strlen("quit"),0);
			printf("退出界面\n");
			clear_lcd();
			break;
		}
	}
	close(sockid);
}

int main()
{
	index_oper();
	
	return 0;
}