#include "stdio.h"
#include "string.h"
#include "opt_test.h"
#include "sys/socket.h"
#include "errno.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "unistd.h"
#include "stdlib.h"
#include "server_init.h"

//int socketInit(char *server_ip,int *strP);

/*
int main(int argc,char **argv)
{
	char server_ip[15] = {0};
	int  strP = 0;

       	argumentParse(argc,argv,server_ip,&strP);
	printf("socketInit before strI = %s strP = %d\n",server_ip,strP);
       if(socketInit(server_ip,&strP) < 0)
       {
	       exit(0);
       }

       return 0;
}
*/
int socketInit(char *server_ip,int *strP)
{
	int sockfd = -1;
	int rv = -1;

       	int opt =  1;

	int port = *strP;
	struct sockaddr_in server_addr;

	/*创建套接字*/
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	printf("sockFd value:%d\n",sockfd);
	if(sockfd == -1)
	{
		printf("create socket error:%s\n",strerror(errno));
		return -1;
	}

	/*将本地ipv4地址和端口填入文件描述符表中*/
	memset(&server_addr,0,sizeof(server_addr));
        server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	printf("port = %d\n",port);
	printf("server_addr.sin_port value :%d\n",server_addr.sin_port);
	printf("socketInit strI = %s\n",server_ip);

	if(server_ip[0] != 0)
	{
		if(inet_pton(AF_INET,server_ip,(void *)&server_addr.sin_addr) < 0) //将点分十进制ipv4地址转换为网络字节序地址
		{
			printf("ipv4 tranform error:%s\n",strerror(errno));
			close(sockfd);
			return -1;
		
		}
		else
		{
			printf("server_addr.sin_addr.s_addr:%x\n",server_addr.sin_addr.s_addr);
		}
	
	}

	else
	{
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//指定绑定任意ipv4地址	
	}


	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&opt,sizeof(opt)) < 0)//设置端口复用，防止被占用
	{
		printf("set port reuse error:%s\n",strerror(errno));
		close(sockfd);
		return -1;
	}

	if(bind(sockfd,(const struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
	{
		printf("bind ip port error:%s\n",strerror(errno));
		close(sockfd);
		return -1;
	}
	else
	{
		printf("bind ip port successfuly\n");
	}

	/*监听socket，等待客户端连接触发并指定服务器可排队连接的最大数*/		
        rv  = listen(sockfd,CONNECTNUM);
	if(rv < 0)
	{
		printf("listen server socket error:%s\n",strerror(errno));
		close(sockfd);
		return -1;
	}

	return sockfd;

}
