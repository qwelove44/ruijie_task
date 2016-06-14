/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * ech_genl_usr.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-24
 * 
 * ech generic netlink socket的客户端
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

#include "ech_client.h"

#define ECH_VERSION    	0x1

#define ECH_FMLY_NAME	"ECH_FAMILY"
#define GENLMSG_DATA(glh)       ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define NLA_DATA(na)            ((void *)((char *)(na) + NLA_HDRLEN))

/*
 * Commands sent from userspace
 * Not versioned. New commands should only be inserted at the enum's end
 * prior to __GENL_DEMO_CMD_MAX
 */
enum {
    ECH_CMD_UNSPEC = 0, /* Reserved */
    ECH_CMD_SET,      /* user->kernel request/get-response */
    __ECH_CMD_MAX,
};

/* User to kernel message attributes */
enum {
    ECH_ATTR_UNSPEC = 0,
    ECH_ATTR_MSG,
    __ECH_ATTR_MAX,
};

typedef struct ech_msgtemplate {
    struct nlmsghdr n;
    struct genlmsghdr g;
    char msg[MAX_MSG_SIZE];                /*  自定义的消息或者数据开始 */
} ech_msgtemplate_t;

/**
 * ech_usr_create_socket - 创建socket并bind
 *
 * 成功返回客户端socket，失败返回-1
 */
static int ech_usr_create_socket(void)
{
    int sock = 0;
    struct sockaddr_nl local;

    sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock < 0) {
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;

    if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

/**
 * ech_usr_send_msg - 通过generic netlink给内核发送数据
 *
 * @sd: 客户端socket
 * @nlmsg_type: family_id
 * @nlmsg_pid: 客户端pid
 * @genl_cmd: 命令类型
 * @genl_version: genl版本号
 * @nla_type: netlink attr类型
 * @nla_data: 发送的数据
 * @nla_len: 发送数据长度
 *
 * 成功返回0，失败返回-1
 */
static int ech_usr_send_msg(int sd, u_int16_t nlmsg_type, u_int32_t nlmsg_pid, u_int8_t genl_cmd,
        u_int8_t genl_version, u_int16_t nla_type,  void *nla_data, int nla_len)
{
    struct nlattr *na;
    struct sockaddr_nl nladdr;
    int r, buflen;
    char *buf;
    ech_msgtemplate_t *msg;

    if (nlmsg_type == 0) {
        return 0;
    }

    msg = (ech_msgtemplate_t *)malloc(NLMSG_LENGTH(GENL_HDRLEN) + NLMSG_ALIGN(nla_len + 1 + NLA_HDRLEN));
    if (msg == NULL) {
        return -1;
    }

    msg->n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    msg->n.nlmsg_type = nlmsg_type;
    msg->n.nlmsg_flags = NLM_F_REQUEST;
    msg->n.nlmsg_seq = 0;
    msg->n.nlmsg_pid = getpid();
    msg->g.cmd = genl_cmd;
    msg->g.version = genl_version;
    na = (struct nlattr *) GENLMSG_DATA(msg);
    na->nla_type = nla_type;
    na->nla_len = nla_len + 1 + NLA_HDRLEN;
    memcpy(NLA_DATA(na), nla_data, nla_len);
    msg->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    buf = (char *)msg;
    buflen = msg->n.nlmsg_len;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    while ((r = sendto(sd, buf, buflen, 0, (struct sockaddr *) &nladdr, sizeof(nladdr))) < buflen) {
        if (r > 0) {
            buf += r;
            buflen -= r;
        } else if (errno != EAGAIN) {
            free(msg);
            return -1;
        }
    }

    free(msg);
    return 0;
}

/**
 * ech_usr_get_family_id - 向服务器请求family id
 *
 * @sd: 客户端socket
 * @family_name: family名字
 *
 * 成功返回id，失败返回-1
 */
static int ech_usr_get_family_id(int sd, char *family_name)
{
    ech_msgtemplate_t *ans;
    struct nlattr *na;
    int id, rc, rep_len;
    char msg[MAX_MSG_SIZE];

    rc = ech_usr_send_msg(sd, GENL_ID_CTRL, 0, CTRL_CMD_GETFAMILY, ECH_VERSION,
            CTRL_ATTR_FAMILY_NAME, (void *)family_name, strlen(family_name) + 1);
    if (rc != 0) {
        return -1;
    }

    rep_len = recv(sd, msg, MAX_MSG_SIZE, 0);
    if (rep_len < 0) {
        return -1;
    }

    ans = (ech_msgtemplate_t *)msg;
    if (ans->n.nlmsg_type == NLMSG_ERROR) {
        return -1;
    }

    rc = NLMSG_OK((&ans->n), rep_len);
    if (!rc) {
        return -1;
    }

    na = (struct nlattr *)GENLMSG_DATA(ans);
    na = (struct nlattr *)((char *) na + NLA_ALIGN(na->nla_len));
    id = -1;
    if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
        id = *(u_int16_t *)NLA_DATA(na);
    }

    return id;
}

/**
 * ech_usr_send_msg - 发送信息接口
 * @msg: 消息指针
 * @msg_len: 消息长度
 *
 * 成功返回0，失败返回-1
 */
int ech_usr_send_pkg(void *msg, int msg_len)
{
    int rc;

    rc = ech_usr_send_msg(g_client_sd, g_client_family_id, 0, ECH_CMD_SET,
            ECH_VERSION, ECH_ATTR_MSG, msg, msg_len);
    if (rc != 0) {
        printf("send cmd %d failed, return %d!", ECH_ATTR_MSG, rc);
        return -1;
    }

    return 0;
}


/**
 * ech_usr_rcv_msg - 接收信息接口
 * @fid: familyid
 * @sock: 套接字
 * @string: 接收数据的buf
 *
 * 成功返回0，失败返回-1
 */
int ech_usr_rcv_msg(int fid, int sock, char *recv_buf)
{
    int ret;
    ech_msgtemplate_t msg;
    struct nlattr *na;

    ret = recv(sock, &msg, sizeof(msg), 0);
    if (ret < 0) {
        return -1;
    }

    if (msg.n.nlmsg_type == NLMSG_ERROR || !NLMSG_OK((&msg.n), ret)) {
        return -1;
    }
    if (msg.n.nlmsg_type == fid && fid != 0) {
        na = (struct nlattr *) GENLMSG_DATA(&msg);
        strcpy(recv_buf, (char *)NLA_DATA(na));
        free(na);
        na=NULL;
    }

    return 0;
}

/**
 * ech_usr_init - generic netlink客户端代码初始化接口
 *
 * 成功返回0，失败返回-1
 */
int ech_usr_init(void)
{
    g_client_sd = ech_usr_create_socket();
    if (g_client_sd == -1) {
        printf("create socket failed!\n");
        goto err;
    }

    g_client_family_id = ech_usr_get_family_id(g_client_sd, ECH_FMLY_NAME);
    if (g_client_family_id == -1) {
        close(g_client_sd);
        printf("get family id failed!\n");
        goto err;
    }

    return 0;

err:

    return -1;
}

/**
 * ech_usr_exit - generic netlink客户端代码删除接口
 *
 * 无返回值.
 */
void ech_usr_exit(void)
{
    close(g_client_sd);

    return;
}

