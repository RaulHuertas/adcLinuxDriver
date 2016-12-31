#ifndef ADCLINUXDRIVER_H
#define ADCLINUXDRIVER_H
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

//ESTRUCTURA CON LOS DATOS INTERNOS DEL DRIVER
struct rghpadc_dev {
    struct miscdevice miscdev;
    void __iomem *direccionRegistros;
    u32 ultimoValorADC;
	u32 ultimaConfig;
	unsigned long paginaAUsar;
	dma_addr_t direccionDMA;
	int irqReportado;
	unsigned long buff_tail;
	unsigned long buff_head;
	unsigned long buff_size;
	unsigned long burst_size;
	spinlock_t buff_lock;
};

//Decaracion de funciones
int rghpadc_probe(struct platform_device *pdev);
int rghpadc_remove(struct platform_device *pdev);
ssize_t rghpadc_read(struct file *file, char *buffer, size_t len, loff_t *offset);
ssize_t rghpadc_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
int rghpadc_mmap(struct file *filp, struct vm_area_struct *vma);
irqreturn_t controlInterrupciones(int irq, void* dev_id); 
long rghpadc_ioctl( struct file* filp, unsigned int cmd, unsigned long arg);


#endif //ADCLINUXDRIVER_H
