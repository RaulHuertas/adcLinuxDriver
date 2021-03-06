#ifndef ADCLINUXDRIVERAPI_H
#define ADCLINUXDRIVERAPI_H
#include <asm/ioctl.h>
#define RGHPADC_IOC_MAGIC  'r'
#define RGHPADC_CAMBIAR_PUERTO		_IOW(RGHPADC_IOC_MAGIC, 0, unsigned int)
#define RGHPADC_CHANGEADDRESS 		_IOW(RGHPADC_IOC_MAGIC,  1, unsigned int)
#define RGHPADC_CLEAN_INTERRUPT 	_IOW(RGHPADC_IOC_MAGIC,  2, unsigned int)
#define RGHPADC_LEEARVALORADC 		_IOR(RGHPADC_IOC_MAGIC,  3, unsigned int)
#define RGHPADC_INICIAR_CAPTURA  	_IOW(RGHPADC_IOC_MAGIC,  4, unsigned int)
#define RGHPADC_REINICIAR  			_IOW(RGHPADC_IOC_MAGIC,  5, unsigned int)
#define RGHPADC_MEJORDIMLECTURA		_IOR(RGHPADC_IOC_MAGIC,  6, unsigned int)
#define RGHPADC_DIMTOTAL			_IOR(RGHPADC_IOC_MAGIC,  7, unsigned int)

#define RGHPADC_IOC_MAXNR 8


#endif //ADCLINUXDRIVERAPI_H
