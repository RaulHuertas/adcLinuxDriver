#include "adcLinuxDriver.h"

static long rghpadc_ioctl( struct file* filp, unsigned int cmd, unsigned long arg){
	if(
		(cmd<0) ||
		(cmd>3)
	){
		return -EINVAL;
	}
	return 0;
}
