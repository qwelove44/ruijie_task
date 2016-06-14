/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * Echo_server_u.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 服务器相关函数
 *
 * History
 */
#include <linux/stat.h>
#include "echo_server_u.h"

#define CMD_CNT 3                /* 存放输入命令的单词个数 */
#define CMD_LEN 10               /* 存放输入命令的单词长度 */
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/* commands sig */
enum {
    CMD_ONE = 0,  /* mode sig */
    CMD_TWO,      /* client_id */
    CMD_THREE,    /* mode */
};

/*字母转换为小写*/
static void ech_tolow_str(char *str, int len){
    int i;
    for (i = 0; i < len; ++i){
        if (str[i] <= 'Z' && str[i] >= 'A'){
            str[i] = 'a' + str[i] - 'A';
        }
    }
    return ;
}
/*字母转换为大写*/
static void ech_toup_str(char *str, int len){
    int i;
    for (i = 0; i < len; ++i){
        if (str[i] <= 'z' && str[i] >= 'a'){
            str[i] = 'A' + str[i] - 'a';
        }
    }
    return;
}
/*字符串处理*/
static void ech_string_prcs(char *str, int len, int flag){
    switch (flag){
    case TOLOW:ech_tolow_str(str, len);break;
    case TOUP:ech_toup_str(str, len);break;
    case NORMAL:break;
    default:printf("the flag is error, please config right flag!\n");
    }
    return;
}

/*查看服务器信息*/
static void ech_show(void)
{
    printf("\nServer info:\n");
    printf("    Global mode: %s\n", flag_cmd.cmd);
    printf("    Characters: %d\n", count_char);
    printf("    Clients: %d\n", count_client);
    return;
}

/*分离字符串*/
static  void ech_split_str(char* cmd, int len, char *str[CMD_CNT])
 {
     int i, j;
     int spce_cnt, str_cnt;

     j = 0;
     spce_cnt = 0;
     str_cnt = 0;
     for (i = 0; i < len; ++i){
         if (cmd[i] == ' '){
            if (str_cnt != 0) {
                strncpy(str[j], cmd + i - str_cnt, str_cnt);
                str[j++][str_cnt] = '\0';
                str_cnt = 0;
            }
            spce_cnt++;
        }else{
            ++str_cnt;
            spce_cnt = 0;
        }
     }
     if (str_cnt){
         int start;
         start = len - spce_cnt - str_cnt;
         strncpy(str[j], cmd + start, str_cnt);
         str[j][str_cnt] = '\0';
     }
     return;
 }

/**
 * ech_config - 服务器配置例程
 *
 * @argv: 空指针类型
 *
 * 返回为空
 */
void *ech_config(void *argv)
 {
     char cmd_str[MAX_MSG_SIZE];
     char *str[CMD_CNT];
     int i;
     bzero(cmd_str, sizeof (cmd_str));
     for (i = 0; i < CMD_CNT; ++i){
         str[i] = (char *)malloc(sizeof (char) * CMD_LEN);
         if (str[i] == NULL){
             perror("malloc failed!");
             int j;
             for (j = 0; j < i; ++j){
                 free(str[j]);
             }
        ech_pool_destroy();
        remove(LOCKPATH);
        exit(1);
         }
         memset(str[i], 0, CMD_LEN);
     }

     while (1){
         int len;
         len = 0;
         printf("SERVER> ");
         if (fgets(cmd_str, MAX_MSG_SIZE, stdin) == NULL){
             perror("input string failed!");
             continue;
         }
         len = strlen(cmd_str);
         cmd_str[len-1] = '\0';
         ech_split_str(cmd_str, len-1, str);

         if ( !strcmp(str[0], "quit")){
            for (i = 0; i < CMD_CNT; ++i){
                free(str[i]);
            }
            ech_pool_destroy();
            remove(LOCKPATH);
            exit(0);
         }else if ( !strcmp(str[CMD_ONE], "ech_show")){
            ech_show();
         }else if ( !strcmp(str[CMD_ONE], "mode")){
            flag_cmd.client_id = atoi(str[CMD_TWO]);
             if ( !strcmp(str[CMD_THREE], "normal")){
                 flag_cmd.flag = 0;
             }else if ( !strcmp(str[CMD_THREE], "lower")){
                 flag_cmd.flag = 1;
             }else if ( !strcmp(str[CMD_THREE], "upper")){
                 flag_cmd.flag = 2;
             }else{
                 perror("please input right cmd!");
             }
        }
         } /* end of while */

     return NULL;
 }

/*线程处理函数*/
static void *ech_thread_process(void* argv)
{
    while (1){
        ech_worker_t *member;
        member = NULL;
        pthread_mutex_lock(&(thread_pool->queue_lock));

        while (thread_pool->cur_queue_num == 0 && thread_pool->shutdown != 1){
            pthread_cond_wait(&(thread_pool->queue_ready), &(thread_pool->queue_lock));
        }

        if (thread_pool->shutdown){
            pthread_mutex_unlock(&(thread_pool->queue_lock));
            pthread_exit(NULL);
        }

        thread_pool->cur_queue_num--;
        member = thread_pool->queue_head;
        thread_pool->queue_head = thread_pool->queue_head->next;
        pthread_mutex_unlock(&(thread_pool->queue_lock));
        (*(member->ech_process))(member->argv);
        free(member);
        member = NULL;
    }
    pthread_exit(NULL);
}
/**
 * ech_pool_init - 初始化线程池
 *
 * @max_num: 线程数
 *
 * 返回为空
 */
void ech_pool_init(int max_num)
{
    int i;

    thread_pool = (ech_thr_pl_t *)malloc(sizeof (ech_thr_pl_t));
    if (thread_pool == NULL){
        perror("malloc failed");
        ech_pool_destroy();
        exit(1);
    }
    thread_pool->max_thread_num = max_num;
    pthread_mutex_init(&(thread_pool->queue_lock), NULL);
    pthread_cond_init(&(thread_pool->queue_ready), NULL);
    thread_pool->cur_queue_num = 0;
    thread_pool->shutdown = 0;
    thread_pool->queue_head = NULL;
    thread_pool->thread_id = (pthread_t *)malloc(max_num * sizeof (pthread_t));
    if (thread_pool->thread_id == NULL){
        perror("malloc failed");
        ech_pool_destroy();
        exit(1);
    }
    for (i = 0;i < max_num;++i){
        pthread_create(&(thread_pool->thread_id[i]), NULL, ech_thread_process, NULL);
    }
    return;
}

/**
 * ech_pool_add_worker - 向线程池添加任务
 *
 * @ech_process: 线程处理例程
 * @argv: 传入例程的参数
 *
 * 成功返回0，失败返回-1
 */
int ech_pool_add_worker(void *(*ech_process) (void *argv), void *argv)
{
    ech_worker_t *member;
    ech_worker_t *newworker;
    member = NULL;
    newworker = (ech_worker_t *)malloc(sizeof (ech_worker_t));
    if (newworker == NULL){
        perror("molloc failed ");
        return -1;
    }
    newworker->ech_process = ech_process;
    newworker->argv = argv;
    newworker->next = NULL;

    pthread_mutex_lock(&(thread_pool->queue_lock));
    member = thread_pool->queue_head;
    if (member != NULL){
        while (member->next){
            member = member->next;
        }
        member->next = newworker;
    }else{
        thread_pool->queue_head = newworker;
    }
    thread_pool->cur_queue_num++;
    pthread_mutex_unlock(&(thread_pool->queue_lock));
    pthread_cond_signal(&(thread_pool->queue_ready));
    return 0;
}

/**
 * ech_pool_destroy - 销毁线程池
 *
 * 成功返回0，失败返回-1
 */
int ech_pool_destroy(void)
{
    int i;

    if (thread_pool->shutdown){
        return -1;
    }
    thread_pool->shutdown = 1;

    /* 唤醒所有等待线程 */
    pthread_cond_broadcast(&(thread_pool->queue_ready));

    /* 阻塞等待线程退出 */
    for (i = 0; i < thread_pool->max_thread_num; ++i){
        pthread_detach(thread_pool->thread_id[i]);
    }
    free(thread_pool->thread_id);

    /* 销毁等待队列 */
    while (thread_pool->queue_head != NULL){
        ech_worker_t *member;
        member = NULL;
        member = thread_pool->queue_head;
        thread_pool->queue_head = member->next;
        free(member);
    }

    /* 销毁互斥量与条件变量 */
    pthread_mutex_destroy(&(thread_pool->queue_lock));
    pthread_cond_destroy(&(thread_pool->queue_ready));

    free(thread_pool);
    thread_pool = NULL;
    return 0;
}

/* 发送和接收消息例程 */
void *ech_process(void *argv)
{
    int sockfd;
    ech_msg_t buf;
    int ret;
    int len;
    int flag;

    sockfd = *(int*)argv;
    len = 0;
    flag = 0;
    
    free(argv);
    while (1){
        ret = recv(sockfd, &buf, sizeof (buf), 0);
        if (ret < 0){
            perror("recv failed!");
            break;
        }
        if (ret == 0){
            printf("\nclient disconnect %u\n", buf.pid);
            break;
        }

        len = strlen(buf.msg);
        count_char += len;

        if (flag_cmd.client_id == 0 || flag_cmd.client_id == buf.pid){
            flag = flag_cmd.flag;
        }
        ech_string_prcs(buf.msg, len, flag);
        usleep(DELAY);

        send(sockfd, &buf, MAX_MSG_SIZE, 0);
    } /* end of while */

    close(sockfd);

    return NULL;
}

/*文件锁*/
static int ech_lockfile(int fd)
{
        struct flock fl;

        fl.l_type  =  F_WRLCK;  /*定义一个独占锁 */
        fl.l_start  =  0;
        fl.l_whence  =  SEEK_SET;
        fl.l_len  =  0;         /*锁住整个文件*/
        fl.l_pid = getpid();

        return(fcntl(fd, F_SETLK, &fl));
}

/**
 * ech_already_running - 确保只能开启一个服务器进程
 *
 * @filename: 加锁的文件
 *
 * 已经存在则返回1，不存在返回0。
 */
int ech_already_running(const char *filename)
{
        int fd;
        fd = 0;
        /* 打开文件 */
        fd  =  open(filename, O_RDWR | O_CREAT, LOCKMODE);
        if (fd  <  0) {
            return -1;
        }

        /* 先获取文件锁 */
        if (ech_lockfile(fd)  ==  -1) {
            if (errno  ==  EACCES || errno  ==  EAGAIN) {
                close(fd);
                return -1;
            }
            exit(1);
        }

        return 0;
}
