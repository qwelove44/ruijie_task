/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * echo_server_k.h
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 内核态服务器头文件
 *
 * History
 */

#ifndef ECHO_SERVER_K_H_
#define ECHO_SERVER_K_H_

#define PROC_DIR            "myproctest"
#define NORMAL              0
#define TOLOW               1
#define TOUP                2

#define MAX_MSG_SIZE        128
#define MAX_THREAD_NUM      5

#define CMD_CNT             3               /* 存放输入命令的单词个数 */
#define CMD_LEN             10              /* 存放输入命令的单词长度 */

#define ECH_NAME            "ECH_FAMILY"
#define ECH_NAME_LEN        32
#define ECH_VERSION         0x1

typedef struct server_info{
    int count_client;
    int count_char;
    char mod[CMD_LEN];
} server_info_t;

typedef struct thread_para{
    char string[MAX_MSG_SIZE];
    int pid;
}thread_para_t;

typedef struct{
    int flag;
    int client_id;
    char cmd[MAX_MSG_SIZE];
}ech_flag_t;

/*
 * Commands sent from userspace
 * Not versioned. New commands should only be inserted at the enum's end
 * prior to __GENL_DEMO_CMD_MAX
 */
enum {
    ECH_CMD_UNSPEC = 0, /* Reserved */
    ECH_CMD_SET,        /* user->kernel request/get-response */
    ECH_CMD_MAX_E,
};
#define ECH_CMD_MAX    (ECH_CMD_MAX_E - 1)

/* User to kernel message attributes */
enum {
    ECH_ATTR_UNSPEC = 0,
    ECH_ATTR_MSG,
    ECH_ATTR_MAX_E,
};
#define ECH_ATTR_MAX   (ECH_ATTR_MAX_E - 1)

#endif /* ECHO_SERVER_K_H_ */
