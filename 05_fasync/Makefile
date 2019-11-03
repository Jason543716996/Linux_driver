
obj-m   := irq_key.o

KERNEL  := /home/zhangjie/linux-4.4

all:
	make -C $(KERNEL) M=`pwd` modules
clean:
	make -C $(KERNEL) M=`pwd` clean 
