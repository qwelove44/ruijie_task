/*
 * main.c
 *
 *  Created on: Jun 14, 2016
 *      Author: yonghua
 */

#include "string_split.h"

#define MAX_LEN 128

int main(int argc, char **argv)
{
	FILE *fp_in,*fp_out;
	char *file_in = "input.txt";
	char *file_out = "output.txt";
	char buf[MAX_BUF];
	struct list_head list = {0};

	INIT_LIST_HEAD(&list);

	/* open file */
	fp_in = fopen(file_in,"r");
	fp_out = fopen(file_out,"w");
	if(NULL==fp_in||NULL==fp_out){
		printf("open file failed!\n");
		exit(1);
	}
	/* 分离所有的单词,大写字母转为小写，存储在链表all_word中 */
	while(fgets(buf,MAX_BUF,fp_in)!=NULL)
		split_word(buf,&list);

	/* 对链表排序并将统计信息输出到指定文件 */
	sort_out(&list,fp_out);

	/* 关闭文件 */
	fclose(fp_in);
	fclose(fp_out);

	return 0;
}


