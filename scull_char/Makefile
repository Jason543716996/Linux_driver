
obj-m	:= demo1.o

KERNEL_DIR := /home/zhangjie/study/Linux_drv/linux-3.5


all:
	make -C $(KERNEL_DIR) M=`pwd` modules

clean:
	make -C $(KERNEL_DIR) M=`pwd` clean 
	



