obj-m:=echo_server_k.o

KERNELBUILD :=/lib/modules/$(shell uname -r)/build
.PHONY: all
all: 
	make -C $(KERNELBUILD) M=$(shell pwd) modules
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions

