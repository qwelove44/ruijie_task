/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * main.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 服务器用户态主函数
 *
 * History
 */

#include "echo_server_u.h"

ech_thr_pl_t *thread_pool = NULL;
int count_char = 0;
int count_client = 0;
ech_flag_t flag_cmd = {0, "normal", 0};

/* 用户态服务器主函数 */
int main(int argc, char **argv) {
    int listen_fd;
    int *connect_fd;
    int ret, cur_cli_num, i;
    pthread_t thread_id;
    struct sockaddr_un server_addr;
    
    connect_fd = NULL;
    /* 设置文件锁，确保只有一个服务器运行 */
    if (ech_already_running(LOCKPATH)){
        printf("server have already run!\n");
        return 0;
    }

    ech_pool_init(MAX_THREAD_NUM);                          /* 初始化线程池 */
    pthread_create(&thread_id, NULL, ech_config, NULL);     /* 配置服务器线程 */

    /* 建socket */
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen < 0){
        perror("build socket is failed");
        goto err;
    }

    /* 绑定服务器地址 */
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, PATH);
    unlink(PATH);
    ret = 0;
    ret = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof (server_addr));
    if (ret < 0){
        perror("bind failed!");
        goto err;
    }

    /* 监听客户端的连接 */
    ret = listen(listen_fd, LISTENUM);
    if (ret < 0){
        perror("listen failed!");
        goto err;
    }

    /* 与客户端通讯 */
    while (1){
        connect_fd = (int *)malloc(sizeof(int));
        if (connect_fd == NULL){
            perror("malloc failed!");
            goto err;
        }
        *connect_fd = accept(listen_fd, NULL, NULL);
        if (*connect_fd < 0){
            perror("accept failed!");
            goto err;
        }

        ret = ech_pool_add_worker(ech_process, connect_fd);
        if (ret < 0){
            printf("add worker failed\n");
        }
        ++count_client;
    }

    return 0;
err:
    close(listen_fd);
    free(connect_fd);
    ech_pool_destroy();
    remove(LOCKPATH);
    return -1;
}
