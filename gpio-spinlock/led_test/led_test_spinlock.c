#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <string.h>

#define GPIOS 2

static char userdata[] = {"user data"};

int main(int argc , char **argv){
	int fd,ret;
	char *filename;
	unsigned char databuf[1];

	if(argc != 3){
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
	databuf[0] = atoi(argv[2]);
	printf("input cmd is %d\n", databuf[0]);

	ret = write(fd, databuf, sizeof(databuf));
	if(ret < 0){
		printf("write cmd failed\n");
	}

	ret = close(fd);
	if(ret < 0){
		printf("can't close file %s\n", filename);
		return -1;
	}
	printf("led test end\n");
	return 0;
}
