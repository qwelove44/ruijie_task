/*
 * main.c
 *
 *  Created on: 2016年5月18日
 *      Author: yonghua
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct worker
{
	void* argv;
	void *(*process) (void *argv);
	struct worker* next;
} worker_t;

typedef struct
{
	pthread_cond_t queue_ready;
	pthread_mutex_t queue_lock;
	pthread_t *thread_id;
	int max_thread_num;
	int cur_queue_num;
	int shutdown;
	worker_t* queue_head;
}Thread_pool_t;

static Thread_pool_t* thread_pool=NULL;

void pool_init(int num);
int pool_destroy();
int pool_add_worker(void* (*process) (void* argv), void* argv);
void* thread_process(void* argv);

/*初始化线程池函数*/
void pool_init(int num)
{
	int i;
	thread_pool=(Thread_pool_t*)malloc(sizeof(Thread_pool_t));
	thread_pool->max_thread_num=num;
	pthread_mutex_init(&(thread_pool->queue_lock),NULL);
	pthread_cond_init(&(thread_pool->queue_ready),NULL);
	thread_pool->cur_queue_num=0;
	thread_pool->shutdown=0;
	thread_pool->queue_head=NULL;
	thread_pool->thread_id=(pthread_t*)malloc(num*sizeof(pthread_t));
	for(i=0;i<num;++i){
		pthread_create(&(thread_pool->thread_id[i]),NULL,thread_process,NULL);
	}
}

/*向线程池添加任务函数*/
int pool_add_worker(void* (*process) (void* argv), void* argv)
{
	worker_t *tmp=NULL;
	worker_t *newworker=(worker_t*)malloc(sizeof(worker_t));
	newworker->process=process;
	newworker->argv=argv;
	newworker->next=NULL;

	pthread_mutex_lock(&(thread_pool->queue_lock));
	tmp=thread_pool->queue_head;
	if(tmp!=NULL){
		while(tmp->next){
			tmp=tmp->next;
		}
		tmp->next=newworker;
	}else{
		thread_pool->queue_head=newworker;
	}
	thread_pool->cur_queue_num++;
	pthread_mutex_unlock(&(thread_pool->queue_lock));
	pthread_cond_signal(&(thread_pool->queue_ready));
	return 0;
}

/*销毁线程池函数*/
int pool_destroy()
{
	int i;
	worker_t *tmp=NULL;

	if(thread_pool->shutdown)
	{
		return -1;
	}
	thread_pool->shutdown=1;

/*	唤醒所有等待线程*/
	pthread_cond_broadcast(&(thread_pool->queue_ready));

	/*阻塞等待线程退出*/
	for(i=0;i<thread_pool->max_thread_num;++i){
		pthread_join(thread_pool->thread_id[i],NULL);
	}
	free(thread_pool->thread_id);

	/*销毁等待队列*/
	while(thread_pool->queue_head!=NULL){
		tmp=thread_pool->queue_head;
		thread_pool->queue_head=tmp->next;
		free(tmp);
	}

	/*销毁互斥量与条件变量*/
	pthread_mutex_destroy(&(thread_pool->queue_lock));
	pthread_cond_destroy(&(thread_pool->queue_ready));

	free(thread_pool);
	thread_pool=NULL;
	return 0;
}

/*线程处理函数*/
void* thread_process(void* argv)
{

	printf("starting thread %lu\n", pthread_self());
	while(1){
		worker_t *tmp=NULL;
		pthread_mutex_lock(&(thread_pool->queue_lock));

		while(thread_pool->cur_queue_num==0&&thread_pool->shutdown!=1){
			printf("thread %lu is waiting\n",pthread_self());
			pthread_cond_wait(&(thread_pool->queue_ready), &(thread_pool->queue_lock));
		}

		if(thread_pool->shutdown){
			pthread_mutex_unlock(&(thread_pool->queue_lock));
			printf("thread %lu will exit\n",pthread_self());
			pthread_exit(NULL);
		}

		printf("thread %lu is starting to work\n",pthread_self());

		thread_pool->cur_queue_num--;
		tmp=thread_pool->queue_head;
		thread_pool->queue_head=thread_pool->queue_head->next;
		pthread_mutex_unlock(&(thread_pool->queue_lock));
		(*(tmp->process))(tmp->argv);
		free(tmp);
		tmp=NULL;
	}
	pthread_exit(NULL);
}

/*工作回调函数*/
void* proces(void* argv)
{
	printf("threadid is %lu, working on task %d\n",pthread_self(),*(int*)argv);
	sleep(1);
	return NULL;
}

int main(int argc, char **argv) {
	int *worknum=(int*)malloc(sizeof(int)*10);
	int i;

	pool_init(5);
	for(i=0;i<10;++i){
		worknum[i]=i+20;
		pool_add_worker(proces,&worknum[i]);
	}
	sleep(3);
	pool_destroy();

	free(worknum);
	worknum=NULL;
	return 0;
}
