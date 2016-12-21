#include <linux/mm.h>
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device-mapper.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/mempool.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include "adcLinuxDriver.h"



//ESTRUCTURA CON INFORMACION SOBRE QUE TIPO DE DISPOSITIVOS USAMOS
static struct of_device_id rghpadc_dt_ids[] = {
    {
        .compatible = "rghpadc,rghpltc2308adc"
    },
    { /* end of table */ }
};
//INFORMAR AL KERNEL QUE TIPO DE DISPOSITIVOS SOPORTAMOS
MODULE_DEVICE_TABLE(of, rghpadc_dt_ids);
//INFORMAMOS EL ROL DE CADA FUNCION EN NUESTRO DRIVER
static struct platform_driver rghpadc_platform = {
    .probe = rghpadc_probe,
    .remove = rghpadc_remove,
    .driver = {
        .name = "rghpadc driver",
        .owner = THIS_MODULE,
        .of_match_table = rghpadc_dt_ids
    }
};
//LAS OPERACIONES QUE PUEDEN REALZIARSE EN ESTOS DISPOSITIVOS
static const struct file_operations rghpadc_fops = {
    .owner = THIS_MODULE,
    .read = rghpadc_read,
    .write = rghpadc_write,
	.mmap = rghpadc_mmap,
	.unlocked_ioctl = rghpadc_ioctl
};
//funcion invocada cuando el driver es inicializado
static int rghpadc_init(void)
{
    int ret_val = 0;
    pr_info("Inicializando el modulo rghpadc...\n");

    //Nuestor modul oes del tipo "Platform Driver",
	//asi que lo registramos como tal
    ret_val = platform_driver_register(&rghpadc_platform);
    if(ret_val != 0) {
        pr_err("platform_driver_register retorno %d\n", ret_val);
        return ret_val;
    }

    pr_info("Modulo rghpadc inicializado exitosamente!\n");

    return 0;
}
//funcion invocada cuando el dispositivo a usar es encontrado
int rghpadc_probe(struct platform_device *pdev)
{
	int ret_val = -EBUSY;
    struct rghpadc_dev *dev;
    struct resource *r = 0;
	int mapeadoRegistros_Size;
	int interruptEnableRet;
	pr_info("rghpadc_probe ingreso\n");
	//OBTENER LOS RECURSOS DE MEMORIA PARA ESTE DISPOSITIVO
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }
	mapeadoRegistros_Size = r->end-r->start+1;
	pr_info( "Size of regs memory map: %d\n", mapeadoRegistros_Size);
	pr_info( "Size of PAGE_SIZE: %d\n",(int) PAGE_SIZE);
	//CREAR LA ESTRUCTURA CON LOS DATOS DEL DRIVER
	dev = devm_kzalloc(&pdev->dev, sizeof(struct rghpadc_dev), GFP_KERNEL);
	//OBTENEMOS LA LINEA DE INTERRUPCION A USAR
	dev->irqReportado = irq_of_parse_and_map(pdev->dev.of_node, 0);
	pr_info( "irqReportado: %d\n", dev->irqReportado);
	//REMAPEAR LOS REGISTROS A UNA SECCION ACCESIBLE
	//POR EL KERNEL LINUX
	dev->direccionRegistros = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->direccionRegistros))
        goto bad_ioremap;
	//INICIALIZAR EL VALOR ADC LEIDO
	dev->ultimoValorADC = 0x0000U;
	//REGISTRAR NUESTRO CONTROLADOR COMO "MISC"
	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = "rghpadc";
	dev->miscdev.fops = &rghpadc_fops;
	ret_val = misc_register(&dev->miscdev);
	if(ret_val != 0) {
        pr_info("No se pudo registrar el dispositivo misc :(");
        goto bad_exit_return;
    }
	//REGISTRAR NUESTRA ESTRUCTURA CON NUESTRO DRIVER
	platform_set_drvdata(pdev, (void*)dev);
	//RESERVAR LA PAGINA  A USAR
	dev->paginaAUsar = __get_free_pages( GFP_USER|GFP_KERNEL, 8);
	if(dev->paginaAUsar==0){
		pr_info("No se pudo pedir buffer para el dispositivo");
		ret_val =  -EPERM ;
        goto bad_exit_return;
	}
	pr_info( "Memoria para la comunicacion pedida con exito\n");
	pr_info( "Direccion devuelta: %u\n", (unsigned int)dev->paginaAUsar);
	pr_info( "Direccion pasada por __pa(): %u\n", (unsigned int)__pa(dev->paginaAUsar));
	pr_info( "Direccion pasada por virt_to_phys(): %u\n", (unsigned int)virt_to_phys(dev->paginaAUsar));
	dev->direccionDMA  = dma_map_single(&pdev->dev, dev->paginaAUsar, PAGE_SIZE, DMA_FROM_DEVICE);
	if (dma_mapping_error(&pdev->dev, dev->direccionDMA)) {
		pr_info("No se pudo realizar mapeado de memoria");
		ret_val =  -EPERM ;
		goto bad_exit_return;
	}
	pr_info( "Direccion para usar dada por dma_map_single(): %u\n", (unsigned int)dev->direccionDMA);
	iowrite32( dev->direccionDMA, ((unsigned int*)dev->direccionRegistros)+1 );
	//INIIALIZAR INTERRUPCIONES
	interruptEnableRet = request_irq(dev->irqReportado, controlInterrupciones, IRQF_NO_SUSPEND, "rghpadc", dev);
	if(interruptEnableRet){
		pr_info("No se pudo inicializar la interrupcion %d, error: %d \n", dev->irqReportado, interruptEnableRet);
	}

	pr_info("rghpadc_probe, salida correcta :)\n");
	return 0;

bad_ioremap:
   ret_val = PTR_ERR(dev->direccionRegistros); 
bad_exit_return:
    pr_info("rghpadc_probe finalizo con errroes :(\n");
    return ret_val;
}

//OPERACIONES DE LECTURA
ssize_t rghpadc_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
	int success = 0;
	struct rghpadc_dev *dev = container_of(file->private_data, struct rghpadc_dev, miscdev);
	//dma_sync_single_for_cpu(dev->miscdev.this_device, dev->direccionDMA, PAGE_SIZE, DMA_FROM_DEVICE);
	//dma_unmap_single(dev->miscdev.this_device, dev->direccionDMA, PAGE_SIZE, DMA_FROM_DEVICE);
	//dev->adc_value = *((unsigned int*)(dev->paginaAUsar));
	//success = copy_to_user(buffer, &dev->adc_value, sizeof(dev->adc_value));
	dev->ultimoValorADC = ioread32(dev->direccionRegistros);
	success = copy_to_user(buffer, &dev->ultimoValorADC, sizeof(dev->ultimoValorADC));
	if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }
	return len;
}
//IMPLEMENTANDO MMAP
int rghpadc_mmap(struct file *filp, struct vm_area_struct *vma){
	
	//struct mm_struct *mm = current->mm;
	int resultadoRemap;
	struct rghpadc_dev* dev;

	pr_info("rghpadc_mmap(), invocado\n");
	dev = (struct rghpadc_dev*)container_of(filp->private_data, struct rghpadc_dev, miscdev);
	resultadoRemap = remap_pfn_range(vma, vma->vm_start, dev->direccionDMA>>PAGE_SHIFT, PAGE_SIZE, vma->vm_page_prot);
	pr_info("remap_pfn_range() invocado\n");
	if (resultadoRemap!=0){
		return -EAGAIN;
	}
	pr_info("rghpadc_mmap(), fin invocacion\n");
    return 0;
}




//WRITE(configurar)
ssize_t rghpadc_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    return len;
}
//CUANDO EL DISPOSITIVO SE PIERDE(nunca en el fpga...)
int rghpadc_remove(struct platform_device *pdev)
{
	struct rghpadc_dev *dev = (struct rghpadc_dev*)platform_get_drvdata(pdev);
	pr_info("rghpadc_remove enter\n");
	free_irq(dev->irqReportado, dev);
	free_pages( dev->paginaAUsar, 0 );
	misc_deregister(&dev->miscdev);
	pr_info("rghpadc_remove exit\n");
    return 0;
}

//CUANDO EL DRIVER SE DESINTALA(rmmod, apagando la pc)
static void rghpadc_exit(void)
{
    pr_info("rghpadc module exit\n");
    platform_driver_unregister(&rghpadc_platform);
    pr_info("rghpadc module successfully unregistered\n");
}

//FUNCION INVOCADA CUANDO HAY UNA INTERRUPCION
irqreturn_t controlInterrupciones(int irq, void* dev_id){
	struct rghpadc_dev* dev = (struct rghpadc_dev*)dev_id;
	iowrite32( 0xFFFFFFFFU, ((unsigned int*)dev->direccionRegistros)+2 );
	//printk( KERN_INFO "Interrupcion recibida\n");
	return IRQ_HANDLED;
}


//LE DECIMOS AL KERNELCUALES SON LAS FUNCIONES DE INICIALIZACION
//Y FINALIZACION DEL DRIVER
module_init(rghpadc_init);
module_exit(rghpadc_exit);
//METADATOS SOBRE ESTE MODULO DEL KERNEL
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Raul Huertas <rax2003_7@hotmail.com>");
MODULE_DESCRIPTION("Driver de un conversor ADC");
MODULE_VERSION("1.0");







