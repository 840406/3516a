KERN_DIR = /work/Hi3516A_SDK_V1.0.7.0/osdrv/opensource/kernel/linux-3.4.y
all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= adv7619.o
