/*
 * client.c
 *
 *  Created on: 2016年5月16日
 *      Author: yonghua
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>

#define PATH "/home/yonghua/workspace/Test_tsocket/share_file"
#define BUFSIZE 500

int main(int argc, char** argv)
{
	struct sockaddr_un addr;
	int sockfd;
	int ret;
	int count=0;
	char buf[BUFSIZE];

	if((sockfd=socket(AF_UNIX, SOCK_STREAM,0))<0){
		perror("build sock failed!\n");
	}
	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path, PATH);
	ret=connect(sockfd,(struct sockaddr*)&addr, sizeof(addr));
	if(ret<0){
		perror("connect error!\n");
	}
	while(1){
		int size=0;
		memset(buf,0,sizeof(buf));
		sprintf(buf,"msg is %d",count++);
		size=send(sockfd,buf,strlen(buf),0);
		printf("size is %d send to: %s\n",size,buf);
		sleep(1);
	}

	return 0;
}

