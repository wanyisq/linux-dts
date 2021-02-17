#include "stdio.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "linux/ioctl.h"
#include "string.h"

#define GPIOS 2

#define CLOSE_CMD	(0x01)
#define OPEN_CMD	(0x02)
#define SETPERIOD	(0x03)

int main(int argc , char **argv){
	int fd,ret;
	char *filename;
	unsigned int cmd, arg;
	unsigned char str[100];

	if(argc != 2){
		printf("error uasge\n");
		return -1;
	}

	filename = argv[1];
	printf("input file name %s\n", filename);
	fd = open(filename, O_RDWR);
	if(fd < 0){
		printf("can't open file %s\n", filename);
		return -1;
	}
	while(1){
		printf("please input cmd:\n");
		ret = scanf("%d", &cmd);
		if(ret != 1){
			fgets(str, sizeof(str), stdin);
		}

		if(cmd == 1){
			cmd = CLOSE_CMD;
		}else if(cmd == 2){
			cmd = OPEN_CMD;
		}else{
			cmd = SETPERIOD;
			printf("input timer period:\n");
			ret = scanf("%d", &arg);
			if(ret != 1){
				fgets(str, sizeof(str), stdin);
			}
		}
		ioctl(fd, cmd, arg);
	}

	ret = close(fd);
	if(ret < 0){
		printf("can't close file %s\n", filename);
		return -1;
	}
	printf("led test end\n");
	return 0;
}
