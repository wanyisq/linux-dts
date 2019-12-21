APP = led_test

CROSS_COMPILE = /home/w/qemu/source/100ask_imx6ull-qemu/ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc

all:
	$(CC) $(APP).c  -o  $(APP) -static

clean:
	rm -f $(APP)
