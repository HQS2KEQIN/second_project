#ifndef _NEEDHFILE_
#define _NEEDHFILE_

typedef struct tlink
{
	char str1[300];
	char str2[300];
	struct tlink *next;
}tl,*TL;
//文件的复制
int copy_file(char *str1,char *str2);
TL head_init();
//目录的复制
int open_dir(TL head,char *src_dir,char *dest_dir);


#endif