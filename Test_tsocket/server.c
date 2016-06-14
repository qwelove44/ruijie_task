/*
 * server.c
 *
 *  Created on: 2016年5月16日
 *      Author: yonghua
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdlib.h>

#define PATH "/home/yonghua/workspace/Test_tsocket/share_file"
#define BUFSIZE 500

struct sockid{
	int sock_o;
	int sock_c;
};
void str_pro(char* str,int len)
{
	int i=0;
	for(i=0;i<len;++i){
		if(str[i]<='z'&&str[i]>='a'){
			str[i]='A'+str[i]-'a';
		}
	}
}

void* fun1(void* argv)
{
	struct sockid sockfd=*(struct sockid*)argv;
	char buf[BUFSIZE];
	int size=0;
	printf("fun1 sock_c is:%d,sock_o is: %d\n",sockfd.sock_c,sockfd.sock_o);
//	pthread_detach(pthread_self());
	while(1){
		size=recv(sockfd.sock_c,buf,BUFSIZE,0);
		if(size<=0){
			if(size==0){
				printf("client unlink\n");
			}
			break;
		}
		printf("recv from :%s\n",buf);
		str_pro(buf,strlen(buf));
		printf("%s\n",buf);
		send(sockfd.sock_c, buf, BUFSIZE, 0);
	}

	return NULL;
}
int main(int argc, char **argv)
{
	struct sockaddr_un saddr,caddr;
	int sockfd,con_sock;
	int ret,len=0;
	char buf[BUFSIZE]="";
	int err;
	struct sockid sfd;
	pthread_t fd;

	sockfd=socket(AF_UNIX, SOCK_STREAM, 0);
	if(sockfd<0){
		perror("builed socket failed!\n");
	}
	bzero(&saddr,sizeof(saddr));
	saddr.sun_family=AF_UNIX;
	strcpy(saddr.sun_path,PATH);
	unlink(PATH);
	ret=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if(ret<0){
		perror("bind failed!\n");
	}else{
		printf("bind is ok!\n");
	}
	sfd.sock_o=sockfd;
	ret=listen(sockfd,5);
	if(ret<0){
		perror("not listen\n");
	}
	while(1){
		int size=0;
//		ret=listen(sockfd,5);
		con_sock=accept(sockfd,(struct sockaddr*)&caddr, &len);

			if(con_sock<0){
				perror("not client!\n");
			}else{
				sfd.sock_c=con_sock;
				err=pthread_create(&fd, NULL, fun1,&sfd);
				if(err!=0){
					perror("create thread failed\n");
				}
			}
			sleep(1);
	}

	return 0;
}

