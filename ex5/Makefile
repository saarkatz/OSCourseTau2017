# Here the name of the target is added.
obj-m := message_module.o
# Here dependencies are added. This has to have the name <target>-objs 
message_module-objs := ./message_slot.o ./message_list.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
