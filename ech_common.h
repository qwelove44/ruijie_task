/*

 * Copyright(C) 2016 Ruijie Network. All rights reserved.


 * ech_common.h
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-24
 *
 * echo generic netlink 公共头文件


#ifndef __ECH_COMMON_H__
#define __ECH_COMMON_H__

 NETLINK_GENERIC related info
#define ECH_NAME       "ETH_PARSER"
#define ECH_NAME_LEN   32
#define ECH_VERSION    0x1

 User to kernel message attributes
enum {
    ECH_ATTR_UNSPEC = 0,
    ECH_ATTR_PKT_DBG,
    __ECH_ATTR_MAX,
};
#define ECH_ATTR_MAX   (__ECH_ATTR_MAX - 1)


 * Commands sent from userspace
 * Not versioned. New commands should only be inserted at the enum's end
 * prior to __GENL_DEMO_CMD_MAX

enum {
    ECH_CMD_UNSPEC = 0,  Reserved
    ECH_CMD_SET,       user->kernel request/get-response
    ECH_CMD_NOTIFY,   kernel->user event
    __ECH_CMD_MAX,
};

#define ECH_CMD_MAX    (__ECH_CMD_MAX - 1)

#endif   __ECH_COMMON_H__
*/
