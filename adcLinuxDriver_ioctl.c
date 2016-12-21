#include <linux/irq.h>
#include <linux/platform_device.h>
#include "adcLinuxDriver.h"
#include "adcLinuxDriverAPI.h"
#include <asm/uaccess.h>

long rghpadc_ioctl( struct file* filp, unsigned int cmd, unsigned long arg){
	
	int err = 0, tmp;
	int retval = 0;
	u32 valorAEscribir = 0;
	u32 actualConfig = 0;
	struct rghpadc_dev* dev;

	if (_IOC_TYPE(cmd) != RGHPADC_IOC_MAGIC) return -ENOTTY;
 	if (_IOC_NR(cmd) > RGHPADC_IOC_MAXNR) return -ENOTTY;
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	dev = (struct rghpadc_dev*)container_of(filp->private_data, struct rghpadc_dev, miscdev);
	 
	switch(cmd){
		case RGHPADC_CAMBIAR_PUERTO:{
			retval = __get_user(valorAEscribir, (u32 __user *)arg);
			valorAEscribir &= 0x07U;
			dev->ultimaConfig = valorAEscribir;
			iowrite32( valorAEscribir, ((unsigned int*)dev->direccionRegistros)+0 );
			break;
		}
		case RGHPADC_CHANGEADDRESS:{
			retval = __get_user(valorAEscribir, (u32 __user *)arg);
			iowrite32( retval, ((unsigned int*)dev->direccionRegistros)+1 );
			break;
		}
		case RGHPADC_CLEAN_INTERRUPT:{
			retval = __get_user(valorAEscribir, (u32 __user *)arg);
			iowrite32( retval, ((unsigned int*)dev->direccionRegistros)+2 );
			break;
		}
		case RGHPADC_LEEARVALORADC:{
			dev->ultimoValorADC = ioread32( ((u32*)dev->direccionRegistros)+0 );
			__put_user(dev->ultimoValorADC, (u32 __user *)arg);
			break;
		}
		default: { /* redundant, as cmd was checked against MAXNR */
    		return -ENOTTY;
		}
	}
	return retval;
}
