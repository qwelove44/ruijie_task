/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * ech_server_k.h
 * Original Author: yonghua, 2016年5月26日
 * 
 */

#ifndef ECH_SERVER_K_H_
#define ECH_SERVER_K_H_

#define BUFSIZE 500

#define ECH_NAME       "ECH_FAMILY"
#define ECH_NAME_LEN   32
#define ECH_VERSION    0x1




#define ECH_CMD_MAX    (__ECH_CMD_MAX - 1)

int ech_knl_init(void);
void ech_knl_exit(void);

#endif /* ECH_SERVER_K_H_ */
