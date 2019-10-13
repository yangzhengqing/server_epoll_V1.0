#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "getopt.h"
#include "string.h"
#include "opt_test.h"

//测试参数解析函数。
/*
int main(int argc , char** argv )
{

//	int opt = -1;
	char *strI;
	char *strP;
	
	argumentParse(argc,argv,strI,strP);
  
	return 0;
		
}
*/

void showHelpMessage(char* argv)
{
	printf("Usage:%s [options] argument ...\n",argv);
	printf("-i:server ip or null,if there is an ip addree,no spaces\n");
	printf("-p:server port\n");
	printf("-h:help message\n");
}

int argumentParse(int argc , char **argv,char *server_ip,int *strP)
{

	int opt = -1;
	
 	while((opt = getopt(argc,argv,"i::p:h"))!=-1)
	{
	//	printf("opt=%C,optarg=%s,optind=%d,argv[optind]=%s,optopt=%c\n",opt,optarg,optind,argv[optind],optopt);
		switch(opt)
		{
			case 'i':
			//	printf("opt = %c\n",opt);
				if(optarg)
				{
					//*strI = optarg;
					strcpy(server_ip,optarg);
					printf("server_ip=%s\n",server_ip);
				}
				break;
			case 'p':
			//	printf("opt = %c\n",opt);
			       	*strP = atoi(optarg);
				printf("strP=%d\n",*strP);
				if(!*strP)
				{
					showHelpMessage(argv[0]);
					return -1;
				}
				
				break;
			case 'h':
			//	printf("opt = %c\n",opt);
				
			//	printf("argv[0]=%s\n",argv[0]);
				showHelpMessage(argv[0]);
				return -1;
			default:
				showHelpMessage(argv[0]);
				return -1;
		
		}

	}
	if(*strP < 0)
	{
		showHelpMessage(argv[0]);
		return -1;
	}

	return 0;
	
        
}
