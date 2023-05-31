#ifndef _NEEDHFILE_
#define _NEEDHFILE_
//文件的复制
int copy_file(char *str1,char *str2);

//目录的复制
int open_dir(char *src_dir,char *dest_dir,thread_pool *pool);

#endif