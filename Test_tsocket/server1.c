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

#define PATH "/home/yonghua/workspace/Test_tsocket/share_file"
#define BUFSIZE 500

int main(int argc, char **argv)
{
	struct sockaddr_un saddr;
	int sockfd;
	int ret,len;
	char buf[BUFSIZE]="";

	sockfd=socket(AF_UNIX,SOCK_STREAM,0);
	if(sockfd<0){
		perror("builed socket failed!\n");
	}
	saddr.sun_family=AF_UNIX;
	strcpy(saddr.sun_path,PATH);
	unlink(PATH);
	ret=bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	if(ret<0){
		perror("bind failed!\n");
	}else{
		printf("bind is ok!\n");
	}
	ret=listen(sockfd,1);
	if(ret<0){
		perror("not listen\n");
	}else{
		printf("sockfd %d,listen ret is: %d\n",sockfd,ret);
	}

	while(1){
		int size=0;
		ret=accept(sockfd,NULL, NULL);
			if(ret<0){
					perror("not client!\n");
				}
//		memset(buf,0,sizeof(buf));
		size=recv(ret, buf, sizeof(buf),0);
		if(size>0){
			printf("recv from : %s\n",buf);
		}
	}

	return 0;
}


