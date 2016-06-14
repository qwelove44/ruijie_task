/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * client.c
 * Original Author: yonghua, 2016年5月26日
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ech_client.h"

int main(int argc, char *argv[])
{
	char send_buf[MAX_MSG_SIZE];
	char recv_buf[MAX_MSG_SIZE];
	int ret;
    if (ech_usr_init() != 0) {
        printf("genl client init failed!");
        goto err;
    }

    /* 与服务器通讯 */
    	while(1){
    		int len = 0;
    		printf("CLIENT> ");
    		bzero(send_buf, sizeof (send_buf));
    		if(fgets(send_buf, MAX_MSG_SIZE, stdin) == NULL){
    			perror("input string failed!");
    			continue;
    		}
    		len = strlen(send_buf);
    		send_buf[len-1] = '\0';
    		if(strcmp(send_buf, "quit") == 0){
    			break;
    		}

    		/* 发送消息到服务器 */
    		ret = ech_usr_send_pkg(send_buf, len);
    		if(ret < 0){
    			perror("send failed!");
    			break;
    		}
    		/* 接收服务器端消息 */
    		ret = ech_usr_rcv_msg(g_client_family_id, g_client_sd, recv_buf);
    		if(ret < 0){
    			printf("recv failed!\n");
    			goto err;
    		}
    		printf("ECHO: %s\n", recv_buf);
    	} /* end of while */

    return 0;

err:
    ech_usr_exit();
    return -1;
}
