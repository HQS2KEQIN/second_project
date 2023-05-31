#ifndef	_THREAD_POOL_H_
#define _THREAD_POOL_H_

//线程的上限个数和任务节点的上限个数
#define MAX_WAITING_TASKS	1000
#define MAX_ACTIVE_THREADS	20

//任务链表的节点模型
typedef struct task{
	int (*do_task)(char *,char *);
	char *str1;
	char *str2;
	struct task *next;
}tasklink,*TASKlink;

//线程池的结构体thread_pool
typedef struct thread_pool{
	pthread_t *tids;			//线程号数组
	TASKlink task_list;		//任务链表头结点
	pthread_mutex_t lock;		//互斥锁
	pthread_cond_t cond;		//条件变量
	unsigned wait_tasks;		//记录当前任务个数
	unsigned active_threads;	//记录当前的线程数
	bool shutdown;				//判断线程是否关闭的标志,true表示关闭,false表示没关闭
}thread_pool;


//线程池操作函数声明
bool init_pthread_pool(thread_pool *pool,unsigned int thread_number);

//防止死锁的例程函数
void handler(void *arg);

//基本例程函数
void *function(void *arg);

//添加任务
bool add_task(thread_pool *pool,int (*do_task)(char *,char *),char *str1,char *str2);

//取消线程
bool remove_thread(thread_pool *pool,unsigned int remove_number);

//销毁线程池
bool destroy_pool(thread_pool *pool);
#endif