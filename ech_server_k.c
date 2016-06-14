/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * ech_servel_k.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-24
 *
 * echo generic netlink socket(kernel)
 */

//#include <linux/module.h>
#include <linux/types.h>
#include <net/genetlink.h>
#include <linux/genetlink.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>

//#include "ech_common.h"
#include "ech_server_k.h"

#define NORMAL						0
#define TOLOW						1
#define TOUP						2

typedef struct thread_para{
	char string[BUFSIZE];
	int pid;
}thread_para_t;

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
#define ECH_ATTR_MAX   (__ECH_ATTR_MAX - 1)


/* Generic netlink family */
static struct genl_family ech_family = {
    .id = GENL_ID_GENERATE,
	.hdrsize = 0,
    .name = ECH_NAME,
    .version = ECH_VERSION,
    .maxattr = ECH_ATTR_MAX,
};

/* Message oeration */
static struct genl_ops ech_ops = {
    .cmd = ECH_CMD_SET,
    .doit = ech_recv_doit,
};

/*字母转换为小写*/
static void ech_tolow_str(char *str, int len){
	int i;
	for(i = 0; i < len; ++i){
		if(str[i] <= 'Z' && str[i] >= 'A'){
			str[i] = 'a' + str[i] - 'A';
		}
	}
	return ;
}
/*字母转换为大写*/
static void ech_toup_str(char *str, int len){
	int i;
	for(i = 0; i < len; ++i){
		if(str[i] <= 'z' && str[i] >= 'a'){
			str[i] = 'A' + str[i] - 'a';
		}
	}
	return;
}
/*字符串处理*/
static void ech_string_prcs(char *str, int len, int flag){
	switch(flag){
	case TOLOW:ech_tolow_str(str, len);break;
	case TOUP:ech_toup_str(str, len);break;
	case NORMAL:break;
	default:printf("the flag is error, please config right flag!\n");
	}
	return;
}

int thread_string_proc(void *argv)
{
	thread_para_t *msg = (thread_para_t *)argv;
	int flag = 1;

	ech_string_prcs(msg->string, strlen(msg->string), flag);
	genl_msg_send_to_user(msg->string, strlen(msg->string), msg->pid);
	kfree(msg);

	return 0;
}

/* The callback function. it triggered when the generic netlink received data */
int ech_recv_doit(struct sk_buff *skb, struct genl_info *info)
{
    /* doit 没有运行在中断上下文 */
    static int          kthread_num = 0;
    struct nlmsghdr     *nlhdr;
    struct genlmsghdr   *genlhdr;
    struct nlattr       *nlh;
    thread_para_t 		*para;              /* 给线程传递参数的结构体 */

    if (skb == NULL) {
		return -1;
	}

	nlhdr = nlmsg_hdr(skb);
	if (nlhdr == NULL) {
		return -1;
	}

	genlhdr = nlmsg_data(nlhdr);
	if (genlhdr == NULL) {
		return -1;
	}

	nlh = genlmsg_data(genlhdr);
	if (nlh == NULL) {
		return -1;
	}

    /* 配置给新开线程所传的参数 */
    /* para 在线程函数thread_string_proc中释放 */
    para = (thread_para_t  *)kmalloc(sizeof(thread_para_t), GFP_KERNEL);
    para->string = nla_data(nlh);
    para->pid = nlhdr->nlmsg_pid;

    /* 每收到一个字符串开辟一个线程 */
    kthread_run(thread_string_proc, (void *)(para), "kthread %d", kthread_num++);

    return 0;
}

int genl_msg_send_to_user(void *data, int len, pid_t pid)
{
    struct sk_buff *skb;
    size_t size;
    void *head;
    int rc;

    size = nla_total_size(len); /* total length of attribute including padding */

    rc = genl_msg_prepare_usr_msg(ECH_CMD_SET, size, pid, &skb);

    if (rc) {
        return rc;
    }

    rc = genl_msg_mk_usr_msg(skb, ECH_ATTR_PKT_DBG, data, len);

    if (rc) {
        kfree_skb(skb);
        return rc;
    }

    head = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));

    rc = genlmsg_end(skb, head);
    if (rc < 0) {
        kfree_skb(skb);
        return rc;
    }

    rc = genlmsg_unicast(&init_net, skb, pid);
    if (rc < 0) {
        return rc;
    }

    return 0;
}


static inline int genl_msg_mk_usr_msg(struct sk_buff *skb, int type, void *data, int len)
{
    int rc;

    /* add a netlink attribute to a socket buffer */
    if ((rc = nla_put(skb, type, len, data)) != 0) {
        return rc;
    }
    return 0;
}

static inline int genl_msg_prepare_usr_msg(u8 cmd, size_t size, pid_t pid, struct sk_buff **skbp)
{
    struct sk_buff *skb;

    /* create a new netlink msg */
    skb = genlmsg_new(size, GFP_KERNEL);
    if (skb == NULL) {
        return -ENOMEM;
    }

    /* Add a new netlink message to an skb */
    genlmsg_put(skb, pid, 0, &ech_family, 0, cmd);

    *skbp = skb;
    return 0;
}

/**
 * ech_knl_init - generic netlink initialization
 *
 * Returns zero on success, else a negative error code(-1).
 */
int ech_knl_init(void)
{
    int ret = 0;

    ret = genl_register_family(&ech_family);
    if (ret < 0) {
        printk(KERN_ERR"HPARSER: Register generic netlink ops(%d) for family(%s) failed.\n",
                  ech_ops.cmd, ech_family.name);
        return ret;
    }

    ret = genl_register_ops(&ech_family, &ech_ops);
    if (ret < 0) {
        printk(KERN_ERR"HPARSER: Register generic netlink ops(%d) for family(%s) failed.\n",
                  ech_ops.cmd, ech_family.name);
        goto err;
    }

    return 0;

err:
    genl_unregister_family(&ech_family);
    return ret;
}

/**
 * ech_knl_exit - generic netlink exit
 *
 * Returns void.
 */
void ech_knl_exit(void)
{
    genl_unregister_family(&ech_family);

    return;
}
