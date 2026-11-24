# Kbuild-частина
ifneq ($(KERNELRELEASE),)
obj-m := hello.o
ccflags-y += -g		# додати налагоджувальну інформацію

# Звичайна частина
else
KDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD
	cp hello.ko hello.ko.unstripped
	$(CROSS_COMPILE)strip -g hello.ko	# прибрати лише налагоджувальну інформацію

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
	rm -f hello.ko.unstripped

%.s %.i: %.c	# make hello.s замість objdump
	$(MAKE) -C $(KDIR) M=$$PWD $@

endif
