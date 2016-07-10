/*
 * main.c
 *
 *  Created on: Jun 14, 2016
 *      Author: yonghua
 */

#include "buf_interface.h"

#define MAX_LEN 	128
#define MAX_CACHE	512

int main(int argc, char **argv)
{
	short dev, blk_nr;
	nand_block_head_t *tmp_nand_block = NULL;
	char data[MAX_LEN] = "test first";

	if (init_blk_pool(MAX_CACHE) < 0) {
		printf("init error\n");
		return -1;
	}
	dev = 5;
	blk_nr = 15;
	printf("get_from hash block node: dev = %d, blk_nr = %d\n", dev, blk_nr);

	tmp_nand_block = get_from_hash(dev, blk_nr);
	if (tmp_nand_block) {
		printf("find block node from hash: dev = %d, blk_nr = %d\n", tmp_nand_block->dev, tmp_nand_block->blk_nr);
		printf("data: %s\n", tmp_nand_block->blk_data);
	} else {
		printf("get_from lru block node: dev = %d, blk_nr = %d\n", dev, blk_nr);
		tmp_nand_block = get_block(dev, blk_nr);
		if(tmp_nand_block) {
			printf("have get block node from lru\n");
			set_block(tmp_nand_block, data);
		} else {
			printf("not block node in lru\n");
			clear_all();
			return -1;
		}
	}
	printf("again get_from hash block node: dev = %d, blk_nr = %d\n", dev, blk_nr);
	tmp_nand_block = get_from_hash(dev, blk_nr);
	if (tmp_nand_block) {
		printf("find block node from hash: dev = %d, blk_nr = %d\n", tmp_nand_block->dev, tmp_nand_block->blk_nr);
		printf("data: %s\n", tmp_nand_block->blk_data);

		strcpy(data, "test second");
		set_block(tmp_nand_block, data);
		printf("second data: %s\n", tmp_nand_block->blk_data);
	} else {
		printf("error get_from hash block node: dev = %d, blk_nr = %d\n", dev, blk_nr);
	}

	clear_all();
	return 0;
}


