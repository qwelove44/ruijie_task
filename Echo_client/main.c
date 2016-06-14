/*
 * Copyright(C) 2016 Ruijie Network. All rights reserved.
 */
/*
 * main.c
 * Original Author:  liyonghua@ruijie.com.cn, 2016-05-20
 *
 * 客户端主函数
 *
 * History
 */

#include "echo_client.h"

int main(int argc, char **argv)
{
    if (argc - 2 != 0){
        printf("please input 2 argv!\n");
        return -1;
    }
    if (!strcmp(argv[1], "-k")){
        echo_client_k();
    }else if (!strcmp(argv[1], "-u")){
        echo_client_u();
    }else{
        perror("please input right argv!");
        return -1;
    }
    return 0;
}
