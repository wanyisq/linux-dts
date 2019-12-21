#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <string.h>

#define GPIOS 2


int main(int argc , char **argv){
	int fd,num,cmd=2;
	char *hello_node = "/dev/hello_gpio";
	char cmdstr[4] = {0}; 
	
	if((0 != memcmp("led", argv[1], strlen("led"))) || (NULL == argv[1]) ||( NULL == argv[2])){
		printf("input cmd error, the cmd is ledx, x mean num\n");
		return -1;
	}
	memcpy(cmdstr, argv[1], strlen(argv[1]));
	num = atoi(&cmdstr[3]);
	printf("the cmd is: set led[%d], %s\n", num, argv[2]);	
	
	if(0 == memcmp("on", argv[2], strlen("on"))){
		cmd = 0;
		printf("led on\n");
	}else
	{
		cmd = 1;
		printf("led off\n");
	}
	
	
	
/*O_RDWR只读打开,O_NDELAY非阻塞方式*/	
	if((fd = open(hello_node,O_RDWR|O_NDELAY))<0){
		printf("APP open %s failed!\n",hello_node);
	}
	else{
		printf("APP open %s success!\n",hello_node);
		ioctl(fd,cmd,num);
	}
	
	close(fd);
	return 0;
}
