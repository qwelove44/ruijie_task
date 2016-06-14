/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * ech_servel_k.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-24
 *
 * echo generic netlink socket(kernel)
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>

#include <net/genetlink.h>
#include <net/netlink.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include "echo_server_k.h"

/* commands sig */
enum {
    CMD_ONE = 0,  /* mode sig */
    CMD_TWO,      /* client_id */
    CMD_THREE,    /* mode */
};

/* echo内核服务器初始化 */
int ech_knl_init(void);
/* echo内核服务器退出 */
void ech_knl_exit(void);
/* Generic netlink 回调函数 */
int ech_recv_doit(struct sk_buff *skb, struct genl_info *info);
/* Generic netlink 线程函数 */
int thread_string_proc(void *argv);
/* 基于Generic netlink发消息给用户端 */
int genl_msg_send_to_user(void *data, int len, pid_t pid);
/* 构造发给用户态客户端的Generic netlink数据 */
static inline int genl_msg_mk_usr_msg(struct sk_buff *skb, int type, void *data, int len);
/* 准备发给用户态客户端的Generic netlink数据 */
static inline int genl_msg_prepare_usr_msg(u8 cmd, size_t size, pid_t pid, struct sk_buff **skbp);

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
    .flags = 0,
    .doit = ech_recv_doit,
    .dumpit = NULL,
};

struct completion conf_complete;
ech_flag_t flag_cmd = {0, 0, "normal"};
server_info_t serv_info = {0, 0, "normal"};

static char cmd_var[MAX_MSG_SIZE] = "";
static struct proc_dir_entry *myprocdir;
static struct task_struct *conf_task;

/*字母转换为小写*/
static inline void ech_tolow_str(char *str, int len){
    int i;
    for (i = 0; i < len; ++i){
        if (str[i] <= 'Z' && str[i] >= 'A'){
            str[i] = 'a' + str[i] - 'A';
        }
    }
    return ;
}
/*字母转换为大写*/
static inline void ech_toup_str(char *str, int len){
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
    default:printk("the flag is error, please config right flag!\n");
    }
    return;
}

static inline int genl_msg_mk_usr_msg(struct sk_buff *skb, int type, void *data, int len)
{
    int ret;

    /* add a netlink attribute to a socket buffer */
    if ((ret = nla_put(skb, type, len, data)) != 0) {
        return ret;
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

/* Generic netlink 线程函数 */
int thread_string_proc(void *argv)
{
    thread_para_t *para;
    int flag;
    flag = 0;
    para = (thread_para_t *)argv;

    if (flag_cmd.client_id == 0 || flag_cmd.client_id == para->pid){
        flag = flag_cmd.flag;
    }

    ech_string_prcs(para->string, strlen(para->string), flag);
    genl_msg_send_to_user(para->string, strlen(para->string), para->pid);
    kfree(argv);
    mdelay(200);
    do_exit(0);
    return 0;
}

/**
 * ech_recv_doit - 回调函数，在generic netlink收到数据后触发
 *
 * @skb: 要处理的数据
 * @info: generic netlink信息
 *
 * 成功返回id，失败返回-1
 */
int ech_recv_doit(struct sk_buff *skb, struct genl_info *info)
{

    /* doit 没有运行在中断上下文 */
    struct nlmsghdr     *nlhdr;
    struct genlmsghdr   *genlhdr;
    struct nlattr       *nlh;
    thread_para_t       *para;              /* 给线程传递参数的结构体 */

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
    strcpy(para->string, (char *)(nla_data(nlh)));
    para->pid = nlhdr->nlmsg_pid;

    serv_info.count_char += strlen(para->string);
    if (flag_cmd.client_id == 0){
        strcpy(serv_info.mod, flag_cmd.cmd);
    }

    /* 每收到一个字符串开辟一个线程 */

    kthread_run(thread_string_proc, (void *)(para), "kthread %d", serv_info.count_client++);

    return 0;
}

/* 基于Generic netlink发消息给用户端 */
int genl_msg_send_to_user(void *data, int len, pid_t pid)
{
    struct sk_buff *skb;
    size_t size;
    void *head;
    int ret;

    size = nla_total_size(len); /* total length of attribute including padding */

    ret = genl_msg_prepare_usr_msg(ECH_CMD_SET, size, pid, &skb);

    if (ret) {
        return ret;
    }

    ret = genl_msg_mk_usr_msg(skb, ECH_ATTR_MSG, data, len);

    if (ret) {
        kfree_skb(skb);
        return ret;
    }

    head = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));

    ret = genlmsg_end(skb, head);
    if (ret < 0) {
        kfree_skb(skb);
        return ret;
    }

    ret = genlmsg_unicast(&init_net, skb, pid);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

static int atoi_knl(const char *str)
{
    int i, ans;

    i = 0;
    ans = 0;
    while (str[i] != '\0'){
        ans = ans * 10 + str[i] - '0';
        ++i;
    }
    return ans;
}

/*分离字符串*/
static void ech_split_str(char *cmd, int len, char *str[CMD_CNT])
{
    int i, j;
    int spce_cnt;
    int str_cnt;

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
int ech_config(void *argv)
{
    char cmd_str[MAX_MSG_SIZE];
    char *str[CMD_CNT];
    int i;
    memset(cmd_str, 0, sizeof (cmd_str));
    for (i = 0; i < CMD_CNT; ++i){
        str[i] = (char *)kmalloc(sizeof (char) * CMD_LEN, 0);
        if (str[i] == NULL){
            int j;
            printk("kmalloc failed!\n");
            for (j = 0; j < i; ++j){
                kfree(str[j]);
            }
            return -1;
        }
        memset(str[i], 0, CMD_LEN);
    }

    while (!kthread_should_stop()){
        int len;
        strcpy(cmd_str, (char *)argv);
        len = 0;
        len = strlen(cmd_str);
        if (len > 0){
            ech_split_str(cmd_str, len-1, str);
        }

        if ( !strcmp(str[0], "quit")){
            break;
        }else if ( !strcmp(str[CMD_ONE], "mode")){
            flag_cmd.client_id = atoi_knl(str[CMD_TWO]);
            if ( !strcmp(str[CMD_THREE], "normal")){
                flag_cmd.flag = 0;
            }else if ( !strcmp(str[CMD_THREE], "lower")){
                flag_cmd.flag = 1;
             }else if ( !strcmp(str[CMD_THREE], "upper")){
                flag_cmd.flag = 2;
            }else{
                printk("please input right cmd!");
            }
            }

        wait_for_completion(&conf_complete);
    } /* end of while */

    for (i = 0; i < CMD_CNT; ++i){
        kfree(str[i]);
    }
    return 0;
}

/**
 * ech_knl_init - generic netlink initialization
 *
 * Returns zero on success, else a negative error code(-1).
 */
int ech_knl_init(void)
{
    int ret;

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

static int conf_read_proc(char *page, char **start, off_t off,int count, int *eof, void *data)
{
    count = sprintf(page, "%s", (char *)data);
    return count;
}

static int conf_write_proc(struct file *file, const char __user *buffer,
                            unsigned long count, void *data)
{
    if (count > MAX_MSG_SIZE) {
            count = 255;
    }
    copy_from_user(data, buffer, count);
    if (strlen((char*)data) > 0){
        complete(&conf_complete);
    }
    return count;
}

static int info_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    server_info_t *info_tmp;
    info_tmp = (server_info_t *)data;
    count = sprintf(page, "\nServer info:\n  Global mode: %s\n  Characters: %d\nClients: %d\n",
                info_tmp->mod, info_tmp->count_char, info_tmp->count_client);
    return count;
}

/* echo内核服务器初始化 */
static int  __init ech_server_init(void)
{
    int ret;
    struct proc_dir_entry *entry;
    entry = NULL;
    init_completion(&conf_complete);

#ifdef CONFIG_PROC_FS
    myprocdir = proc_mkdir(PROC_DIR, NULL);
    entry = create_proc_entry("config_mod", 0666, myprocdir);
    if (entry) {
            entry->data = (void *)&cmd_var;
            entry->read_proc = &conf_read_proc;
            entry->write_proc = &conf_write_proc;
    }

    entry = create_proc_entry("server_info", 0444, myprocdir);
    if (entry) {
            entry->data = (void *)&serv_info;
            entry->read_proc = &info_read_proc;
            entry->write_proc = NULL;
    }
#else
        printk("This module requires the kernel to support procfs,\n");
#endif

    conf_task = kthread_run(ech_config, cmd_var, "kthread_conf");
    if (IS_ERR(conf_task)){
        printk("kthread_create error/n");
        return -1;
    }
    ret = ech_knl_init();
    if (ret < 0){
        printk("ech_kel_inti failed!\n");
        return -1;
    }

    return 0;
}

/* echo内核服务器退出 */
static void __exit ech_server_exit(void)
{
#ifdef CONFIG_PROC_FS
    remove_proc_entry("server_info", myprocdir);
    remove_proc_entry("config_mod", myprocdir);
    remove_proc_entry(PROC_DIR, NULL);
#endif
    complete(&conf_complete);
    kthread_stop(conf_task);
    ech_knl_exit();
}

module_init(ech_server_init);
module_exit(ech_server_exit);
MODULE_AUTHOR("LIYONGVHUA");
MODULE_DESCRIPTION("ech_server_k");
MODULE_LICENSE("GPL");
