
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define KEY0VALUE 0XF0
#define KEY1VALUE	0xF1
#define INVAKEY 0X00
/*
 * ./keytest /dev/key
 */
int main(int argc, char **argv)
{
	int fd, ret;
	char *filename;
	unsigned char keyvalue;;
	
	/* 1. 判断参数 */
	if (argc != 2) 
	{
		printf("Error Usage!\r\n");
		return -1;
	}

	filename = argv[1];
	/* 2. 打开文件 */
	fd = open(filename, O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	while(1) {
		read(fd, &keyvalue, sizeof(keyvalue));
		if (keyvalue == KEY0VALUE) {
			printf("KEY0 Press, value = %#X\r\n", keyvalue);/* 按下 */
		}else if(keyvalue == KEY1VALUE){
			printf("KEY1 Press, value = %#X\r\n", keyvalue);/* 按下 */
		}
	}
	
	ret = close(fd);
	if(ret < 0){
		printf("file %s close failed\r\n", fd);
		return -1;
	}
	
	return 0;
}


