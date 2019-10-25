#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "errno.h"
#include "sys/epoll.h"
#include "epoll_test.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "opt_test.h"
#include "server_init.h"
#include "fcntl.h"
#include "dup_test.h"

int setnonblocking(int sock);
int readData(int fd,char *buf);
int writeData(int fd,char *buf);
int server_epoll_create(int sockfd);
void handler_read(int fd);
void response_client(int fd);

int main(int argc,char **argv)
{
	int rv = -1;
	char server_ip[15];
	int strP = -1;
	int sockfd = -1;

	server_epoll_log();//将打印信息重定向的日志文件
	printf("welcom use server epoll...\n");
	rv = argumentParse(argc,argv,server_ip,&strP);
	if(rv < 0)
	{
		printf("argumentParse error:%s\n",strerror(errno));
		exit(0);
	}
	sockfd = socketInit(server_ip,&strP);
	if(sockfd < 0)
	{
		printf("socketInit error:%s\n",strerror(errno));
		exit(0);
	}
	rv = server_epoll_create(sockfd);
	if(rv < 0)
	{
		printf("server_epoll_create error:%s\n",strerror(errno));
		close(sockfd);
		exit(0);
	}

	return 0;
}

int server_epoll_create(int sockfd)
{
	int epollfd = -1;
	int connectfd = -1;
	int rvctl = -1;
	int rvwait = -1;
	int rv = -1;
	int client_len;
	int i = 0;

	struct sockaddr_in client_addr;
	struct epoll_event sockEvent;
	struct epoll_event rvEvent[MAXEVENT];

	/*初始化*/
	memset(&client_addr,0,sizeof(client_addr));
	/*创建epoll文件描述符，相当于向内核申请一个文件系统*/
	epollfd = epoll_create(FDNUM);
	if(epollfd < 0)
	{
		printf("epoll create error:%s\n",strerror(errno));
		return -1;
	}
	/*将套接字文件描述符设置为非阻塞*/
	if(setnonblocking(sockfd) < 0)
	{
		printf("setnonblocking error:%s\n",strerror(errno));
		close(sockfd);
		exit(0);

	}

	/*配置sockEvent感性趣的事件*/
	sockEvent.events = EPOLLIN;//设置为读和默认水平触发模式
	sockEvent.data.fd = sockfd;

	/*注册事件*/
	rvctl = epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&sockEvent);
	if(rvctl < 0)
	{
		printf("epoll_ctl error:%s\n",strerror(errno));
		close(epollfd);
		return -1;
	}

	printf("Waiting for client connecting...\n");
	while(1)
	{
		/*等待客户端连接*/
		/*等待事件触发*/
		rvwait = epoll_wait(epollfd,rvEvent,MAXEVENT,-1);//-1:永远阻塞直到有文件描述被触发时返回		
		if(rvwait < 0)
		{
			printf("epoll_wait error:%s\n",strerror(errno));
			close(epollfd);
			return -1;
		}

		else//说明有事件被触发，即有文件描述符可读
		{
			/*查找对应描述符*/
			for(i = 0;i < rvwait;i++)
			{
				if(rvEvent[i].data.fd == sockfd)//新的连接
				{
					connectfd = accept(sockfd,(struct sockaddr *)&client_addr,&client_len);//接收请求
					if(connectfd < 0)
					{
						printf("accept client[%s:%d] error:%s\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port,strerror(errno));
						return -1;				

					}
					else
					{
						printf("client[%s:%d][%d] connecting successfully!\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port,connectfd);
					}

					if(setnonblocking(connectfd) < 0)//将新连接设置为非组塞状态
					{
						printf("setnonblocking connectfd error:%s\n",strerror(errno));
						return -1;
					}
					/*设置新连接上的文件描述符属性*/
					sockEvent.data.fd = connectfd;
					sockEvent.events = EPOLLIN ;//默认触发模式
					epoll_ctl(epollfd,EPOLL_CTL_ADD,connectfd,&sockEvent);//将新的fd添加到epoll监听的队列中
				}
				else//监听队列中有文件描述可读，即有数据从客户端到来
				{
					if(rvEvent[i].events & EPOLLIN)//判断文件描述符属性是否为可读
					{
						/*处理文件描述为负数的情况*/
						if(rvEvent[i].data.fd < 0)
						{
							continue;
						}

						/*从客户端读取数据*/
						handler_read(rvEvent[i].data.fd);
						response_client(rvEvent[i].data.fd);
					}
				}
			}
		}
	}
}


int setnonblocking(int sock)//根据文件描述符操作文件特性
{
	int opts;
	opts = fcntl(sock,F_GETFL);//获取文件标记
	if(opts < 0)
	{
		printf("fcntl F_GETL error:%s\n",strerror(errno));
		return -1;
	}

	opts |= O_NONBLOCK;

	if(fcntl(sock,F_SETFL,opts) < 0)//设置文件标记
	{
		printf("fcntl F_SETL ERROR:%s\n",strerror(errno));
		return -1;
	}

	return 0;
}

/*while循环抱死，非阻塞读写数据，防止边缘触发数据一次读写不完全*/
int readData(int fd,char *buf)
{
	int n = 0;
	int nread = -1;
	int count = 0;
	while(1)
	{
		nread = read(fd,buf + n,BUFSIZE -1);

		if(nread < 0)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				continue;
			}
			else
			{
				return -1;
			}
		}
		else if(nread == 0)
		{
			return -1;
		}	
		else
		{
			n += nread;
			count++;
		}	
	}
	return count++;
}

int writeData(int fd,char *buf)
{
	int data_size = strlen(buf);
	int rv = -1;
	int n = data_size;

	while(n > 0)
	{
		rv = write(fd,buf + data_size - n,n);
		if(rv < n)
		{
			if(rv == -1 && errno != EAGAIN )
			{
				printf("write error:%s\n",strerror(errno));
				close(fd);
				return -1;
			}
			break;
		}

		n -= rv;
	}
}



void handler_read(int fd)
{

	int rv = -1;
	char readBuff[BUFSIZE];

	memset(readBuff,0,sizeof(readBuff));
	rv = recv(fd,readBuff,BUFSIZE -1,0);
	if(rv > 0)
	{
		printf("server receive from client[%d] datas:%s\n",fd,readBuff);
	}
	else if(rv == 0)
	{
		printf("client[%d] disconnected!\n",fd);
		close(fd);
	}
	else if(rv < 0)
	{
		printf("server read from client[%d] data appearing error:%s\n",fd,strerror(errno));
		close(fd);
	}

}

void response_client(int fd)
{
	int rv = -1;
	char buf[BUFSIZE] = "server received datas successfully!";


	rv = send(fd,buf,strlen(buf),0);
	if(rv > 0)
	{
		printf("server send successfully!\n");
	}
	else
	{
		printf("server send failed:%s\n",strerror(errno));
		close(fd);
	}

}
