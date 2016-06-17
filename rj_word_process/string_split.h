/*
 * string_split.h
 *
 *  Created on: Jun 17, 2016
 *      Author: yonghua
 */

#ifndef STRING_SPLIT_H_
#define STRING_SPLIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "list.h"

#define MAX_BUF 512
#define MAX_WORD 50

typedef struct word_list{
	char word[MAX_WORD];
	int cnt;
	struct list_head list;
}word_list, *Pword_list;

void split_word(char* buf,struct list_head* list);
void sort_out(struct list_head* list, FILE *fp_out);

#endif /* STRING_SPLIT_H_ */
