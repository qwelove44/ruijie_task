/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * ech_client.h
 * Original Author: yonghua, 2016年5月26日
 * 
 */

#ifndef ECH_CLIENT_H_
#define ECH_CLIENT_H_

#include <linux/types.h>
#include <sys/types.h>

#define MAX_MSG_SIZE    500

int g_client_sd;         /* socket id*/
int g_client_family_id;  /* 内核分配的通用netlink 协议族 */

int ech_usr_init(void);
void ech_usr_exit(void);
int ech_usr_send_pkg(void *msg, int msg_len);
int ech_usr_rcv_msg(int fid, int sock, char *string);

#endif /* ECH_CLIENT_H_ */
