#ifndef _NEEDHFILE_
#define _NEEDHFILE_
//文件复制
int copy_file(char *str1,char *str2);

//目录复制
int open_dir(char *src_dir,char *dest_dir,thread_pool *pool);

#endif
