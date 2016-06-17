/*
 * string_split.c
 *
 *  Created on: Jun 17, 2016
 *      Author: yonghua
 */

#include "string_split.h"

void split_word(char* buf,struct list_head* list){
	struct list_head *pos=NULL;
	int cn, i;
	int len = strlen(buf);
	cn = 0;
	for(i=0;i<len;++i){

		if(!isalpha(buf[i])){
			Pword_list tmp=(Pword_list)malloc(sizeof(word_list));
			int sig=1;
			if(0==cn) continue;
			memset(tmp->word,0,sizeof(tmp->word));
			tmp->cnt=1;
			strncpy(tmp->word,buf+i-cn,cn);
			list_for_each(pos,list){
				Pword_list st=list_entry(pos,word_list,list);
				if(strcmp(st->word,tmp->word)==0){
					st->cnt++;
					sig=0;
					break;
				}
			}
			if(sig)
				list_add(&tmp->list,list);
			cn=0;
		}
		else{
			++cn;
			if(isupper(buf[i]))
				buf[i]=tolower(buf[i]);
		}
	}
}

static int compar(const Pword_list a,const Pword_list b){
	return strcmp(a->word,b->word);
}

void sort_out(struct list_head* list, FILE *fp_out){
	Pword_list data;
	int i=0,len=0;
	struct list_head *pos=NULL;

	list_for_each(pos,list){
		++len;
	}
	data=(Pword_list)malloc(sizeof(word_list)*len);
	list_for_each(pos,list){
		Pword_list tmp=list_entry(pos,word_list,list);
		data[i++]=*tmp;
	}
	//排序
	qsort(data,len,sizeof(word_list),compar);

	//将排好序的统计数据输出到指定文件。
	for(i=0;i<len;++i){
		fprintf(fp_out,"%s, %d\n",data[i].word,data[i].cnt);
	}
	free(data);
}

