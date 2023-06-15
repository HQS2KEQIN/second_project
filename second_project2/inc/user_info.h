#ifndef _USERINFO_
#define _USERINFO_
typedef struct friend_list{
	char name[30];
	char qq[12];
	int bmp_n;
	struct friend_list *next;
}flink,*Flink;

typedef struct user_info{
	char name[30];
	char qq[12];
	int  connid;
	char passwd[30];
	char phonenum[12];
	char sex[5];
	char states[10];
	int age;
	struct sockaddr_in user_addr;
	struct user_info *next;
}ulink,*Ulink;

typedef struct talk_list{
	char qq[12];
	char name[20];
	char content[100];
	struct talk_list *next;
	struct talk_list *prev;
}tlink,*Tlink;

typedef struct group_list{
	char qq[12];
	char name[20];
	struct group_list *next;
}glink,*Glink;

typedef struct groups_link{
	char groupname[20];
	int mebnum;
	struct groups_link *next;
}gslink,*GSlink;

void register_info();

void gamemeun(char *qq);
	void friendlist();
		void insert_friend_node(char *qq,char *name,int bmp_n);
		Flink init_friend_head();
		void insert_group_node(char *groupname,int mebnum);
		
	void talk_to_friend();
		void insert_tnode(char *qq,char *name,char *content);
	
void get_xy(int *x,int *y,int *p);

void clear_lcd();

int open_lcd();

int sockid;
int block3;
int block4;
int block5;
int block6;
int block7;
int block8;
char head_path[30];
char head_name[20];
char self_qq[12];
Flink fhead;
Tlink thead;
int talk_up;
#endif