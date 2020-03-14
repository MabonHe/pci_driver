obj-m := pcie.o
CONFIG_MODULE_SIG=n
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	gcc oseAPP.c -o oseAPP
clean:
	rm -f *.o *.ko
	rm -f oseAPP
install:
	cp ose_pci.h /usr/local/include/


