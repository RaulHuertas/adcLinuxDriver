#include "adcLinuxDriver.h"
#include <linux/circ_buf.h>
#include <linux/dma-mapping.h>

//FUNCION INVOCADA CUANDO HAY UNA INTERRUPCION
irqreturn_t controlInterrupciones(int irq, void* dev_id){
	struct rghpadc_dev* dev = (struct rghpadc_dev*)dev_id;

	//Actualizar el buffer con los datos
	spin_lock(&dev->buff_lock);
	unsigned long head = dev->buff_head; 
	unsigned long tail = READ_ONCE(dev->buff_tail);
	if( CIRC_SPACE(head, tail, dev->buff_size) >= dev->burst_size ) {//si hay espacio...
		dma_sync_single_for_cpu(
			dev->miscdev.this_device, 
			dev->direccionDMA+head, 
			dev->burst_size, 
			DMA_FROM_DEVICE
		);
		smp_store_release(
			&dev->buff_head,
		  	((head + dev->burst_size) & (dev->buff_size - 1))
		);
	}else{
		printk( KERN_INFO "buffer lleno :(\n");
	}

	spin_unlock(&dev->buff_lock);


	//LIMPIAR EL BIT DE INTERRUPCION
	iowrite32( 0xFFFFFFFFU, ((unsigned int*)dev->direccionRegistros)+2 );
	//printk( KERN_INFO "Interrupcion recibida\n");
	return IRQ_HANDLED;
}


