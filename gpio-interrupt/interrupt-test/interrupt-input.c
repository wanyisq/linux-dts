#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "linux/ioctl.h"
#include <string.h>


int main(int argc , char **argv){
	int fd,ret = 0;
	char *filename;
	unsigned int data = 0;

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
		ret = read(fd, &data, sizeof(data));
		if(ret < 0){

		}else
		{
			if(data){
				printf("key value = %#x\r\n", data);
			}
		}
	}

	ret = close(fd);
	return ret;
}
