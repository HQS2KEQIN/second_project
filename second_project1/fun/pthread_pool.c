#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<stdbool.h>
#include<unistd.h>
#include "pthread_pool.h"

void handler(void *arg)
{
	pthread_mutex_t *lock =(pthread_mutex_t *)arg;
	pthread_mutex_unlock(lock);
} 

void *function(void *arg)
{
	thread_pool *pool = (thread_pool *)arg;
	TASKlink t = NULL;
	while(1)
	{
		pthread_cleanup_push(handler,(void *)&pool->lock);
		pthread_mutex_lock(&pool->lock);
		
		while(pool->wait_tasks==0&&!pool->shutdown)
		{
			pthread_cond_wait(&pool->cond,&pool->lock);

		}
		if(pool->shutdown&&pool->wait_tasks==0)
		{	
			pthread_mutex_unlock(&pool->lock);
			pthread_exit(NULL);
		}
		t = pool->task_list->next;
		pool->task_list->next = t->next;
		t->next = NULL;
		pool->wait_tasks--;
		pthread_mutex_unlock(&pool->lock);
		pthread_cleanup_pop(0);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);	
		t->do_task(t->str1,t->str2);
		free(t);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
		
	}
}

//参数修改处
bool add_task(thread_pool *pool,int (*do_task)(char *,char *),char *str1,char *str2)
{
	pthread_cleanup_push(handler,(void*)&pool->lock);
	pthread_mutex_lock(&pool->lock);
	if(pool->wait_tasks>=MAX_WAITING_TASKS)
	{
		pthread_mutex_unlock(&pool->lock);
		return false;
	}
	
	TASKlink node =(TASKlink)malloc(sizeof(tasklink));

	if (node == NULL)
	{	
		perror("new task create ");
		pthread_mutex_unlock(&pool->lock);
		return false;
	}
	node->do_task = do_task;
	//参数修改处
	node->str1 = str1;
	node->str2 = str2;
	node->next = NULL;
	TASKlink t = pool->task_list;
	while(t->next != NULL)
		t = t->next;
	

	t->next = node;
	pool->wait_tasks++;
	pthread_mutex_unlock(&pool->lock);
	pthread_cleanup_pop(0);
	pthread_cond_signal(&pool->cond);
	return true;
}

bool add_thread(thread_pool *pool,unsigned int add_number)
{
	unsigned int total_thread = pool->active_threads + add_number;
	int i,ret;
	for(i = pool->active_threads;i<total_thread&&i<MAX_ACTIVE_THREADS;i++)
	{
		ret = pthread_create(&pool->tids[i],NULL,function,(void *)pool);
		if(ret!=0)
		{
			perror("add thread fall\n");
			return false;
		}
		pool->active_threads++;
	}
	return true;
	
}

bool remove_thread(thread_pool *pool,unsigned int remove_number)
{
	int deta = pool->active_threads-remove_number;
	if(deta<1)
	{
		perror("remove too more\n");
		deta == 1;
	}
	int i,ret;
	for(i = pool->active_threads-1;i>deta-1;i--)
	{
		pthread_detach(pool->tids[i]);
		pthread_cancel(pool->tids[i]);
		pool->tids[i] = pool->tids[i+1];
		
	}
	pool->active_threads = deta;
	return true;
}

bool init_pthread_pool(thread_pool *pool,unsigned int thread_number)
{
	pool->tids = malloc(sizeof(pthread_t)*MAX_ACTIVE_THREADS);
	pool->task_list = (TASKlink)malloc(sizeof(tasklink));
	if(pool->tids == NULL||pool->task_list==NULL)
	{
		printf("创建失败\n");
		return false;
	}
	pool->task_list->next = NULL;
	pool->wait_tasks = 0;
	pthread_mutex_init(&pool->lock,NULL);	
	pthread_cond_init(&pool->cond,NULL);
	int i = 0;

	for(;i<thread_number;i++)
	{
		int ret = pthread_create(&pool->tids[i],NULL,function,(void *)pool);
		if(ret!=0)
		{
			perror("线程创建失败\n");
			return false;
		}
		
		pool->active_threads++;
	}
	
	pool->shutdown = false;
	
	return true;
}

bool destroy_pool(thread_pool *pool)
{
	//将关闭标志位置为true
	pool->shutdown = true;
	//将所有线程都唤醒
	pthread_cond_broadcast(&pool->cond);
	//接合所有线程
	int i,ret;
	for(i=0;i<pool->active_threads;i++)
	{
		ret = pthread_join(pool->tids[i],NULL);
		if(ret != 0)
			printf("[%u]不许下班\n",(unsigned)pool->tids[i]);
		else
			printf("[%u]下班了\n",(unsigned)pool->tids[i]);
	}
	//释放堆内存
	free(pool->tids);
	free(pool->task_list);
	free(pool);
	
	return true;
}