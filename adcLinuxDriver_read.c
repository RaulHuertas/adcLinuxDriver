#include "adcLinuxDriver.h"
#include <linux/circ_buf.h>


//OPERACIONES DE LECTURA
ssize_t rghpadc_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
	
	int success = 0;
	struct rghpadc_dev *dev = container_of(file->private_data, struct rghpadc_dev, miscdev);
	if(len!=dev->burst_size){
		return -EINVAL;
	}
	spin_lock(&dev->buff_lock);
	unsigned long head = smp_load_acquire(&dev->buff_head);
	unsigned long tail = dev->buff_tail;
	if (CIRC_CNT(head, tail, dev->buff_size) >= dev->burst_size) {
		smp_store_release(
			&dev->buff_tail,
		  	(tail + dev->burst_size) & (dev->buff_size - 1)
		);
		success = dev->burst_size;
	}
	spin_unlock(&dev->buff_lock);
	return success;
}
