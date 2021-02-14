
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * ./ledtest /dev/100ask_led0 on
 * ./ledtest /dev/100ask_led0 off
 */
int main(int argc, char **argv)
{
	int fd;
	char status[2];
	
	printf("argc num is %d\r\n", argc);
	/* 1. 判断参数 */
	if (argc != 4) 
	{
		printf("Usage: %s <dev> num <on | off>\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	/* 3. 写文件 */
	status[0] = (char)atoi(argv[2]);
	if (0 == strcmp(argv[3], "on"))
	{
		
		status[1] = 1;
		write(fd, &status, 2);
	}
	else
	{
		status[1] = 0;
		write(fd, &status, 2);
	}
	printf("led argv0 is %d, argv1 is %d\n", status[0], status[1]);
	
	close(fd);
	
	return 0;
}


