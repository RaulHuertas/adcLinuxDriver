#include <asm/ioctl.h>
#define RGHPADC_IOC_MAGIC  'r'
#define RGHPADC_CONFIG    			_IOW(RGHPADC_IOC_MAGIC, 0)
#define RGHPADC_CHANGEADDRESS 		_IOW(RGHPADC_IOC_MAGIC,  1, int)
#define RGHPADC_CLEAN_INTERRUPT 	_IOW(RGHPADC_IOC_MAGIC,  2, int)
