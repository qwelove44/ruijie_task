/*
 * buf_interface.h
 *
 *  Created on: 2016年7月9日
 *      Author: lyh
 */

#ifndef BUF_INTERFACE_H_
#define BUF_INTERFACE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"

#define BLK_SIZE			512		/* the chip block size */
#define HASH_BITS       	6
#define HASH_SIZE       	(1UL << HASH_BITS)
#define HASH_MASK       	(HASH_SIZE-1)
#define DATA_VALID         1		/* If data is valid */

typedef struct nand_block_head_s {
    struct list_head hash;
    struct list_head lru;
    int    dev;
    int    blk_nr;
    int    flags;					/* see below */
    unsigned char blk_data[0];
} nand_block_head_t;



int init_blk_pool(int num);
nand_block_head_t *get_block(int dev, int blk_nr);

nand_block_head_t *get_from_hash(short dev, short blk_nr);
int set_block(nand_block_head_t *node, const char *data);
void clear_all();

#endif 								/* BUF_INTERFACE_H_ */
