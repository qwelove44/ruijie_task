/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * server_knl.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-24
 *
 * echo generic netlink socket(kernel)
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ech_server_k.h"

static int  __init ech_server_init(void)
{
	int ret = 0;
	ret = ech_knl_init();
	if(ret < 0){
		printk("ech_kel_inti failed!\n");
		return -1;
	}

	return 0;
}

static void  __exit ech_server_exit(void)
{
	ech_knl_exit();
}

module_init(ech_server_init);
module_exit(ech_server_exit);
MODULE_AUTHOR("LIYONGVHUA");
MODULE_DESCRIPTION("ech_server");
MODULE_LICENSE("GPL");
