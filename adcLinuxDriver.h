#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/ioctl.h>
#define RGHPADC_IOC_MAGIC  'r'
#define RGHPADC_CONFIG    			_IOW(RGHPADC_IOC_MAGIC, 0)
#define RGHPADC_CHANGEADDRESS 		_IOW(RGHPADC_IOC_MAGIC,  1, int)
#define RGHPADC_CLEAN_INTERRUPT 	_IOW(RGHPADC_IOC_MAGIC,  2, int)


//Decaracion de funciones
static int rghpadc_probe(struct platform_device *pdev);
static int rghpadc_remove(struct platform_device *pdev);
static ssize_t rghpadc_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t rghpadc_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static int rghpadc_mmap(struct file *filp, struct vm_area_struct *vma);
static irqreturn_t controlInterrupciones(int irq, void* dev_id); 
static long rghpadc_ioctl( struct file* filp, unsigned int cmd, unsigned long arg);



