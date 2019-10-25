#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "errno.h"
#include "dup_test.h"

//测试重定向函数
/*int main(int argc,char **argv)
{
	server_epoll_log();
	printf("hello\n");
	printf("man\n");
	printf("xioabai\n");
	printf("shuhcushbdkvjhsakvb:%d\n",12);
	return 0;
}
*/

int server_epoll_log(void)
{
	int rv = -1;
	int fd = open("./server_epoll_log",O_RDWR|O_CREAT,0777);//创建并打开日志文件
	if(fd < 0)
	{
		printf("open log error");
		return -1;
	}
	else
	{
		printf("open log successfully!\n");
		close(1);
		rv = dup2(fd,1);//文件描述符重定向
		return 0;
	}
}
