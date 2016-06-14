obj-m:=server.o ech_server_k.o

#ech_server_k.o: ech_common.h

KERNELBUILD :=/lib/modules/$(shell uname -r)/build
.PHONY: all
all: 
	 make -C $(KERNELBUILD) M=$(shell pwd) modules
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions

