KDIR ?= ../../linux-socfpga


ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSCULL_DEBUG 
else
  DEBFLAGS = -O2
endif

LDDINC=$(PWD)/../include

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(LDDINC)



default:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR)

clean:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) clean

help:
	$(MAKE) -C $(KDIR) ARCH=arm M=$(CURDIR) help







