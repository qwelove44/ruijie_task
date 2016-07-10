/*
 * buf_interface.c
 *
 *  Created on: 2016年7月9日
 *      Author: lyh
 */

#include "buf_interface.h"

struct list_head blk_head_hashtable[HASH_SIZE];
LIST_HEAD(blk_head_lru);

static int key(short dev, short blk_nr)
{
	int key;
	key = ((dev << 16) | blk_nr);
	return key % BLK_SIZE;
}

int init_blk_pool(int num)
{
	int i;
	nand_block_head_t *tmp_nand_block;

	tmp_nand_block = NULL;

	for (i = 0; i < HASH_SIZE; ++i) {
		INIT_LIST_HEAD(&blk_head_hashtable[i]);
	}
	for (i = 0; i < num; ++i) {
		int key_no;
		key_no = i % HASH_SIZE;
		tmp_nand_block = (nand_block_head_t *)malloc(sizeof(nand_block_head_t) + BLK_SIZE);
		if (tmp_nand_block == NULL){
			perror("malloc error");
			return -1;
		}
		tmp_nand_block->dev = i >> 16;
		tmp_nand_block->blk_nr = i & 0xffff;
		tmp_nand_block->flags = -1;
		list_add(&(tmp_nand_block->lru), &blk_head_lru);
		list_add(&(tmp_nand_block->hash), &blk_head_hashtable[key_no]);
	}
	return 0;
}
nand_block_head_t *get_block(int dev, int blk_nr)
{
	nand_block_head_t *tmp_nand_block;
	struct list_head *pos;
	int key_no;

	if (list_empty(&blk_head_lru)) {
		return NULL;
	}
	tmp_nand_block = NULL;
	pos = blk_head_lru.next;
	tmp_nand_block = list_entry(pos, nand_block_head_t, lru);

	list_del_init(pos);
	list_del_init(&(tmp_nand_block->hash));
	key_no = key(dev, blk_nr);
	list_add(&(tmp_nand_block->hash), &blk_head_hashtable[key_no]);
	if(tmp_nand_block) {
		tmp_nand_block->dev = dev;
		tmp_nand_block->blk_nr = blk_nr;
	}

	return tmp_nand_block;
}
static void put_block(nand_block_head_t *buffer)
{
	buffer->flags = -1;
	list_add_tail(&(buffer->lru), &blk_head_lru);
}

nand_block_head_t *get_from_hash(short dev, short blk_nr)
{
	int key_no;
	nand_block_head_t *node = NULL;
	struct list_head *pos = NULL;

	key_no = key(dev, blk_nr);
	if(list_empty(&blk_head_hashtable[key_no])) {
		return NULL;
	}
	list_for_each(pos, &blk_head_hashtable[key_no]) {
		nand_block_head_t *tmp = NULL;
		tmp = list_entry(pos, nand_block_head_t, hash);
		if(tmp->flags != -1 && tmp->dev == dev && tmp->blk_nr == blk_nr) {
			node = tmp;
			break;
		}
	}

	return node;
}

int set_block(nand_block_head_t *node, const char *data)
{
	if (node) {
		node->flags = DATA_VALID;

		memcpy(node->blk_data, data, strlen(data));
		return 0;
	} else {
		printf("node is null\n");
		return -1;
	}
}

void clear_all()
{
	struct list_head *pos = NULL;
	struct list_head *tmp_list_node = NULL;
	nand_block_head_t *tmp_nand_block = NULL;
	int i;

	for (i = 0; i < HASH_SIZE; ++i) {
		if (list_empty(&blk_head_hashtable[i])) {
			continue;
		}
		list_for_each_safe(pos, tmp_list_node, &blk_head_hashtable[i]) {
			tmp_nand_block = list_entry(pos, nand_block_head_t, hash);
			list_del_init(pos);
			free(tmp_nand_block);
			tmp_nand_block = NULL;
		}
	}/*end of for*/
}
