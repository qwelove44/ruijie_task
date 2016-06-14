/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * echo_server_u.h
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 用户态服务器头文件
 *
 * History
 */

#ifndef ECHO_SERVER_SERVER_U_H_
#define ECHO_SERVER_SERVER_U_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#define NORMAL                      0
#define TOLOW                       1
#define TOUP                        2
#define MAX_MSG_SIZE                128
#define LISTENUM                    5
#define MAX_THREAD_NUM              3
#define DELAY                       200000
#define PATH                        "/home/echo/echo_server/share"
#define LOCKPATH                    "lockpath"

typedef struct ech_worker{
    void *argv;
    void *(*ech_process) (void *argv);
    struct ech_worker *next;
} ech_worker_t;

typedef struct{
    pthread_cond_t queue_ready;
    pthread_mutex_t queue_lock;
    pthread_t *thread_id;
    int max_thread_num;
    int cur_queue_num;
    int shutdown;
    ech_worker_t *queue_head;
}ech_thr_pl_t;

typedef struct{
    char msg[MAX_MSG_SIZE];
    int pid;
}ech_msg_t;

typedef struct{
    int flag;
    char cmd[MAX_MSG_SIZE];
    int client_id;
}ech_flag_t;

extern ech_thr_pl_t *thread_pool;        /* 线程池对象 */
extern ech_flag_t flag_cmd;              /* 服务器全局配置对象 */
extern int count_char;                   /* 服务器处理字符数 */
extern int count_client;                 /* 服务器通信的客户端数目 */

/* 配置函数 */
void *ech_config(void *argv);
/* 线程池初始化 */
void ech_pool_init(int max_num);
/* 向线程池添加任务 */
int ech_pool_add_worker(void *(*ech_process) (void *argv), void *argv);
/* 销毁线程池 */
int ech_pool_destroy(void);
/* 发送与接收消息 */
void *ech_process(void* argv);
/* 文件加锁 */
int ech_already_running(const char *filename);

#endif /* ECHO_SERVER_SERVER_U_H_ */
