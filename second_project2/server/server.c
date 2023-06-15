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
#include "user_info.h"

#define IP "192.168.13.52"
#define PORT 50003
#define MAX_HOST 20

Ulink head;

struct sockaddr_in recv_addr[MAX_HOST];
int flag;
//创建用户信息头节点
Ulink init_head()
{
	Ulink head = (Ulink)malloc(sizeof(ulink));
	if(head == NULL)
	{
		perror("create head fail\n");
		return NULL;
	}
	head->next = NULL;
	return head;
}

Glink init_group_head()
{
	Glink head = (Glink)malloc(sizeof(glink));
	if(head == NULL)
	{
		perror("create head fail\n");
		return NULL;
	}
	head->next = NULL;
	return head;
}

//插入用户信息节点
void insert_node(int connid,struct sockaddr_in recv_ipo,char *recv_acc,char *recv_pn,char *recv_pwd,char *recv_sex,char *recv_sta)
{
	Ulink t = head;
	while(t->next!=NULL)
	{
		if(!strcmp(t->next->qq,recv_acc))
		{
			send(connid,"banrgacc",strlen("banrgacc"),0);
			return ;
		}
		if(!strcmp(t->next->phonenum,recv_pn))
		{
			send(connid,"banrgpn",strlen("banrgpn"),0);
			return ;
		}
		t = t->next;	
	}
	Ulink node = (Ulink)malloc(sizeof(ulink));
	node->user_addr = recv_ipo;
	strcpy(node->phonenum,recv_pn);
	strcpy(node->sex,recv_sex);
	strcpy(node->qq,recv_acc);
	strcpy(node->passwd,recv_pwd);
	strcpy(node->states,recv_sta);
	node->connid = connid;
	node->next = NULL;
	t->next = node;
	send(connid,"accessr",strlen("accessr"),0);
	FILE *file = fopen("../lib/account.txt","a+");
	fprintf(file,"%s %d %s %s %s %s %s\n",inet_ntoa(recv_ipo.sin_addr),recv_ipo.sin_port,recv_acc,recv_pn,recv_pwd,recv_sex,recv_sta);
	fclose(file);
}

//插入存储用户账号信息文件数据
void insert_node_file(char *ip,int port,char *qq,char *phonenum,char *passwd,char *sex)
{
	Ulink t = head;
	while(t->next != NULL)
		t = t->next;
	
	Ulink node = (Ulink)malloc(sizeof(ulink));
	inet_pton(AF_INET,ip,&(node->user_addr.sin_addr));
	node->user_addr.sin_port = htons(port);
	stpcpy(node->qq,qq);
	stpcpy(node->phonenum,phonenum);
	stpcpy(node->passwd,passwd);
	stpcpy(node->sex,sex);
	stpcpy(node->states,"offline");
	node->next = NULL;
	t->next = node;
}

//通过账号寻找用户节点
Ulink node_find_qq(char *qq)
{
	Ulink t = head;
	while(t->next!=NULL)
	{
		if(!strcmp(t->next->qq,qq))
			return t->next;
		
		t = t->next;
	}
	return NULL;
}

//通过套接字寻找用户节点
Ulink node_find_connid(int connid)
{
	Ulink t = head;
	while(t->next!=NULL)
	{
		if(t->next->connid == connid)
			return t->next;
	
		t = t->next;
	}
	return NULL;
}

//插入名字结点
void insert_name(char *qq,char *name)
{
	Ulink t = head;
	while(t->next!=NULL)
	{
		t = t->next;
		if(!strcmp(t->qq,qq))
		{
			strcpy(t->name,name);
			return;
		}
	}
	return ;
}

//判断输入的信息是否正确
void judge(int connid,Ulink node,char *recv_acc,char *recv_pwd)
{
	Ulink t = head->next;
	while(t!=NULL)
	{
		if(!strcmp(t->qq,recv_acc))
		{
			if(!strcmp(t->passwd,recv_pwd))
			{
				strcpy(t->states,"online");
				send(connid,"true",strlen("true"),0);
				node = node_find_qq(recv_acc);
				node->connid = connid;
			}
			
			else
				send(connid,"false passwd",strlen("false passwd"),0);
			
			return ;
		}
		t = t->next;
	}
	send(connid,"false qq",strlen("false qq"),0);
	return ;
}

//客户端连接服务器
int accep_connect()
{	
	int sockid = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_pton(AF_INET,IP,&server.sin_addr);
	bind(sockid,(struct sockaddr *)&server,sizeof(server));
	listen(sockid,MAX_HOST);
	return sockid;
}

//第一次客户登陆信息数据保存
void write_user_info(Ulink node,char *name,int age,int picture)
{
	strcpy(node->name,name);
	node->age = age;
	char path[50] = {0};
	sprintf(path,"../lib/user_info/%s/info.txt",node->qq);
	FILE *file_info = fopen(path,"a+");
	fprintf(file_info,"%s %s %s %s %s %d %d\n",node->qq,node->phonenum,node->sex,node->passwd,name,age,picture);
	fclose(file_info);

}

//创建了信息的客户登陆获取头像和名字
int find_picture(char *path,char *name)
{
	char abpath[50];
	sprintf(abpath,"%s/info.txt",path);
	printf("path = %s\n",abpath);
	
	FILE *file = fopen(abpath,"r+");
	int pic;
	int a;
	char b[12];
	char c[12];
	char d[20];
	char e[6];
	fscanf(file,"%[^\n]\n",abpath);
	sscanf(abpath,"%s %s %s %s %s %d %d\n",b,c,d,e,name,&a,&pic);
	return pic;
}

void insert_gnode(Glink ghead,char *qq,char *name)
{
	Glink t = ghead;
	while(t->next!=NULL)
		t = t->next;
	Glink node = (Glink)malloc(sizeof(glink));
	strcpy(node->qq,qq);
	strcpy(node->name,name);
	node->next = NULL;
	t->next = node;	
}

void write_gnode_file(Glink ghead,char *groupname)
{
	Glink t = ghead;
	char file_path[100];
	while(t->next!=NULL)
	{
		bzero(file_path,sizeof(file_path));
		t = t->next;
		sprintf(file_path,"../lib/user_info/%s/group/%s.txt",t->qq,groupname);
		Glink m = ghead;
		FILE *file = fopen(file_path,"w+");
		while(m->next!=NULL)
		{
			m = m->next;
			fprintf(file,"%s %s\n",m->qq,m->name);
		}
		fclose(file);
	}
	
}

void send_gtalk(Glink head,char *self_qq,char *talk)
{
	Glink t = head;
	while(t->next != NULL)
	{
		t = t->next;
		if(!strcmp(t->qq,self_qq))
			continue;
		Ulink node = node_find_qq(t->qq);
		send(node->connid,talk,strlen(talk),0);
	}		
}

//服务器线程用于接收/发送对应客户端所发送的消息
void *recv_data(void *arg)
{
	struct sockaddr_in recv_ipo = recv_addr[flag];
	int connid = *(int *)arg;
	Ulink self_node = (Ulink)malloc(sizeof(ulink)); 
	char buf[200] = {0};
	char self_qq[12];
	while(1)
	{
		bzero(buf,sizeof(buf));
		recv(connid,buf,sizeof(buf),0);
		printf("buf = %s\n",buf);
		if(!strncmp(buf,"login",5))
		{
			char str[10];
			char recv_acc[12] = {0};
			char recv_pwd[30] = {0};
			sscanf(buf,"%s %s %s",str,recv_acc,recv_pwd);	
			judge(connid,self_node,recv_acc,recv_pwd);
			strcpy(self_qq,recv_acc);
		}
	
		if(!strncmp(buf,"register",8))
		{
			char str[10];
			char recv_acc[12] = {0};
			char recv_pn[12] = {0};
			char recv_pwd[30] = {0};
			char recv_sex[6] = {0};
			char recv_sta[10] = {0};
			sscanf(buf,"%s %s %s %s %s %s",str,recv_acc,recv_pn,recv_pwd,recv_sex,recv_sta);
			insert_node(connid,recv_ipo,recv_acc,recv_pn,recv_pwd,recv_sex,recv_sta);
		}
		
		if(!strncmp(buf,"ifchoose",8))
		{
			char str[10];
			char path[40];
			sscanf(buf,"%s %s",str,path);
			if(access(path,F_OK))
			{
				mkdir(path,0777);
				send(connid,"noexist",strlen("noexist"),0);
				
			}
			else
			{
				char path1[50];
				char name[20];
				int picture = find_picture(path,name);
				strcpy(self_node->name,name);
				insert_name(self_qq,name);
				sprintf(path1,"yesexist ../lib/headp/%d.bmp %s",picture,name);
				send(connid,path1,strlen(path1),0);	
			}
		}
		
		if(!strncmp(buf,"choose",6))
		{
			char str[10];
			char name[20];
			char qq[12];
			int age;
			int picture;
			sscanf(buf,"%s %s %s %d %d",str,qq,name,&age,&picture);
			Ulink node = node_find_qq(qq);
			write_user_info(node,name,age,picture);
		}
		
		if(!strncmp(buf,"onplayer",8))
		{
			char str[100];
			Ulink t = head;
			while(t->next!=NULL)
			{			
				t = t->next;
				if(!strcmp(t->states,"online"))
				{
					bzero(str,sizeof(str));
					sprintf(str,"check %s %s",t->qq,t->name);
					send(connid,str,strlen(str),0);
					usleep(2000);
				}
			}
			usleep(2000);
			send(connid,"listok",strlen("listok"),0);
		}
		
		if(!strncmp(buf,"addplayer",9))
		{
			char str[10];
			char qq[12];
			sscanf(buf,"%s %s",str,qq);
			Ulink node = node_find_qq(qq);
			
			if(node == NULL)
			{
				send(connid,"nofindfri",strlen("nofindfri"),0);
				continue;
			}
			char ack1[50];
			char ack2[50];
			char path1[30];
			char path2[30];
			sprintf(path1,"../lib/user_info/%s",qq);
			sprintf(path2,"../lib/user_info/%s",self_qq);
			int pic1 = find_picture(path1,node->name);
			int pic2 = find_picture(path2,self_node->name);
			sprintf(ack1,"addplayer %s %s %d",node->qq,node->name,pic1);
			send(connid,ack1,strlen(ack1),0);
			sprintf(ack2,"addplayer %s %s %d",self_qq,self_node->name,pic2);
			send(node->connid,ack2,strlen(ack2),0);
		}
	
		if(!strncmp(buf,"ifonline",8))
		{
			char str[10];
			char qq[12];
			char name[30];
			sscanf(buf,"%s %s %s",str,qq,name);
			Ulink node = node_find_qq(qq);
			if(!strcmp(node->states,"offline"))
				send(connid,"noonline",strlen("noonline"),0);
			else
				send(connid,"yesonline",strlen("yesonline"),0);
		}
	
		if(!strncmp(buf,"talkgroup",9))
		{
			
			char str[10],groupname[20];
			char path[100];
			sscanf(buf,"%s %s",str,groupname);
			sprintf(path,"../lib/user_info/%s/group/%s.txt",self_qq,groupname);
			printf("path = %s\n",path);
			FILE *file = fopen(path,"r+");
			if(file == NULL)
			{
				send(connid,"nfgroup",strlen("nfgroup"),0);
				fclose(file);
				continue;
			}
			
			char qq[12],name[20];
			char strr[30];
			
			Glink ghead = init_group_head();
			while(!feof(file))
			{
				fscanf(file,"%[^\n]\n",strr);
				sscanf(strr,"%s %s\n",qq,name);
				insert_gnode(ghead,qq,name);
			}
			
			fclose(file);
			char talk[200];
			while(1)
			{
				
				bzero(buf,sizeof(buf));
				bzero(talk,sizeof(talk));
				recv(connid,buf,sizeof(buf),0);
				if(!strncmp(buf,"quit",4))
					break;
				sprintf(talk,"gtalk %s %s %s %s",groupname,self_qq,self_node->name,buf);
				send_gtalk(ghead,self_qq,talk);
			}
		}
		
		else if(!strncmp(buf,"talk",4))
		{
			char str[10];
			char qq[12];
			char content[100];
			sscanf(buf,"%s %s %s",str,qq,content);
			Ulink node = node_find_qq(qq);
			char talk[200];
			sprintf(talk,"talk %s %s %s",self_qq,self_node->name,content);
			send(node->connid,talk,strlen(talk),0);
		}
		
		if(!strncmp(buf,"file",4))
		{
			char str[5],file_name[50],qq[12];
			sscanf(buf,"%s %s %s",str,file_name,qq);
			Ulink node = node_find_qq(qq);
			char str1[100];
			sprintf(str1,"file %s %s %s",self_qq,self_node->name,file_name);
			send(node->connid,str1,strlen(str1),0);	
			while(1)
			{
				bzero(str1,sizeof(str1));
				recv(connid,str1,sizeof(str1),0);
				if(!strcmp(str1,"sendok"))
				{
					sleep(1);
					send(node->connid,str1,sizeof(str1),0);
					break;
				}
				send(node->connid,str1,strlen(str1),0);

			}

		}
	
		if(!strncmp(buf,"bulidgroup",10))
		{
			Glink ghead = init_group_head();
			char str[20],groupname[20];
			int num;
			sscanf(buf,"%s %s %d",str,groupname,&num);
			char dir_path[100];
			char filename_path[100];
			sprintf(dir_path,"../lib/user_info/%s/group",self_qq);
			if(access(dir_path,F_OK))
				mkdir(dir_path,0777);
			
			sprintf(filename_path,"%s/groupname.txt",dir_path);
			FILE *file2 = fopen(filename_path,"a+");
			fprintf(file2,"%s %d\n",groupname,num);
			fclose(file2);
			char qq[12],name[20];
			insert_gnode(ghead,self_qq,self_node->name);
			while(1)
			{
				bzero(filename_path,sizeof(filename_path));
				bzero(dir_path,sizeof(dir_path));
				bzero(qq,sizeof(qq));
				bzero(name,sizeof(name));
				bzero(buf,sizeof(buf));
				
				
				recv(connid,buf,sizeof(buf),0);
				if(!strncmp(buf,"finish",6))
					break;
				sscanf(buf,"%s %s",qq,name);
				insert_gnode(ghead,qq,name);
				sprintf(dir_path,"../lib/user_info/%s/group",qq);
				if(access(dir_path,F_OK))
					mkdir(dir_path,0777);
			
				sprintf(filename_path,"%s/groupname.txt",dir_path);
				FILE *filename = fopen(filename_path,"a+");
				fprintf(filename,"%s %d\n",groupname,num);
				fclose(filename);
			}
			write_gnode_file(ghead,groupname);
			free(ghead);
		}
	
		if(!strncmp(buf,"opengroup",9))
		{
			char dir_path[100];
			sprintf(dir_path,"../lib/user_info/%s/group",self_qq);
			if(access(dir_path,F_OK))
				send(connid,"nogroup",strlen("nogroup"),0);
			else
			{
				send(connid,"yesgroup",strlen("yesgroup"),0);
				usleep(1000);
				char file_path[50];
				sprintf(file_path,"%s/groupname.txt",dir_path);
				FILE *file = fopen(file_path,"r+");
				char str[50];
				while(!feof(file))
				{  
					bzero(str,sizeof(str));
					fscanf(file,"%[^\n]\n",str);
					send(connid,str,strlen(str),0);
					usleep(1000);
				}
				send(connid,"finish",strlen("finish"),0);
			}
		}
	

		if(!strncmp(buf,"exit",4))
		{
			printf("账号为%s的%s已下线\n",self_qq,self_node->name);
			strcpy(self_node->states,"offline");
		}
		
		if(!strncmp(buf,"quit",4))
			break;
	}
	pthread_exit(NULL);
	
}

int main()
{
	pthread_t tid[MAX_HOST];
	head = init_head();
	int conntid;
	int i = 1;
	FILE *file = fopen("../lib/account.txt","r+");
	char ip[20];
	int port;
	char qq[12];
	char phonenum[12];
	char passwd[30];
	char states[7];
	char buf[100];
	char sex[5];
	int cid;
	while(!feof(file))
	{
		bzero(buf,sizeof(buf));
		fscanf(file,"%[^\n]\n",buf);
		sscanf(buf,"%s %d %s %s %s %s %s\n",ip,&port,qq,phonenum,passwd,sex,states);
		insert_node_file(ip,port,qq,phonenum,passwd,sex);
	}
	fclose(file);
	
	int sockid = accep_connect();
	for(;i<=MAX_HOST;i++)
	{
		int size = sizeof(recv_addr[i-1]);
		printf("等待连接\n");
		flag = i-1;
		conntid = accept(sockid,(struct sockaddr *)&recv_addr[i-1],&size);
		if(conntid != -1)
			pthread_create(&tid[i-1],NULL,recv_data,(void *)&conntid);
	}
	char str[10] = {0};

	
	for(int i = 1;i<MAX_HOST;i++)
	{
		pthread_join(tid[i-1],NULL);
	}
	close(sockid);
	return 0;
}