/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * Echo_client.h
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 客户端头文件
 *
 * History
 */

#ifndef ECHO_CLIENT_H_
#define ECHO_CLIENT_H_

#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <sys/types.h>

#define MAX_MSG_SIZE        128
#define PATH                "/home/echo/echo_server/share"
#define ECH_VERSION         0x1
#define ECH_FMLY_NAME       "ECH_FAMILY"
#define genlmeg_data(glh)       ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define NLA_DATA(na)            ((void *)((char *)(na) + NLA_HDRLEN))

typedef struct
{
    char msg[MAX_MSG_SIZE];
    int pid;
}ech_msg_t;

/* 与用户服务器端通信 */
void echo_client_u(void);
/* 与内核服务器端通信 */
void echo_client_k(void);

#endif  /* ECHO_CLIENT_H_ */
