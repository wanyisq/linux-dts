#!/bin/bash
export ARCH=arm
obj-m += gpio_int.o 

KDIR := /media/psf/qemu-source/100ask_imx6ull-qemu/linux-4.9.88

PWD ?= $(shell pwd)
all:
	make -C $(KDIR) M=$(PWD) modules
		
clean:
	rm *.ko *.o  *.mod.c *.order *.symvers
