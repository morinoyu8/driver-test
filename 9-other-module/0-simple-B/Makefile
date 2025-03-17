CFILES := testdriverB.c
obj-m := testdriverB.o
ccflags-y += -std=gnu99 -Wall -Wno-declaration-after-statement
KBUILD_EXTRA_SYMBOLS := ../0-simple-A/Module.symvers

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean