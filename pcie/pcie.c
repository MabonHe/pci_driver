#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/signal.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/ioctl.h>
#include "ose_pci.h"
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("IPC device driver");

#define DEV_NAME "ose_ipc"
#define DEBUG

#ifdef DEBUG
	#define DEBUG_ERR(format,args...) \
	do{  \
		printk("[ERROR][%s:%d] ",__FUNCTION__,__LINE__); \
		printk(format,##args); \
	}while(0)

	#define DEBUG_PRINT(format,args...) \
	do{  \
		printk("[INFO][%s:%d] ",__FUNCTION__,__LINE__); \
		printk(format,##args); \
	}while(0)
#endif
/*
struct Ose_ioctl{
	unsigned int offset;
	unsigned int val;
};
#define VENDOR_ID 0x8086
#define DEVICE_ID 0x4bb3
#define DEVICE_NUMBER 1
#define DATA_LENGTH 124
#define OSE_MAGIC 0x10
#define OSE_MEM_W _IOW(OSE_MAGIC,0,struct Ose_ioctl)
#define OSE_MEM_R _IOR(OSE_MAGIC,1,struct Ose_ioctl)
*/
unsigned long bar0_phy = 0;
unsigned long bar0_vir = 0;
unsigned long bar0_length;
unsigned long bar1_phy;
unsigned long bar1_vir;
unsigned long bar1_length;
static struct class * ose_class;
static struct device * ose_dev;
static int ose_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void ose_remove(struct pci_dev *dev);
static int ose_open(struct inode *inode,struct file *file);
static long ose_ioctl(struct file *file,unsigned int cmd,unsigned long arg);
static int  ose_release(struct inode *inode,struct file *file);
static ssize_t ose_read(struct file *file,char __user *buf,size_t length,loff_t *loff);
static ssize_t ose_write(struct file *file,const char __user *buf,size_t length,loff_t *loff);
//static loff_t ose_llseek(struct file * filp,loff_t offset, int orig);
static struct pci_device_id ose_ipc_ids[] = { \
    {VENDOR_ID,DEVICE_ID,PCI_ANY_ID,PCI_ANY_ID,0,0,0}, \
    {0,} \
};

struct Ose_device
{
	struct pci_dev* pci_dev;
	struct cdev cdev;
	dev_t devno;
}osedev;
MODULE_DEVICE_TABLE(pci,ose_ipc_ids);



static struct  pci_driver ipc_driver = \
{ \
	.name=DEV_NAME,
	.id_table = ose_ipc_ids,
	.probe = ose_probe,
	.remove = ose_remove,
};
static int ose_open(struct inode *inode,struct file *file)
{
	struct Ose_device *dev;
	dev = container_of(inode->i_cdev,struct Ose_device,cdev);
	file->private_data = dev;
	DEBUG_PRINT(KERN_INFO "open\n");
	return 0;
}

static long ose_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	unsigned int *p;
	struct Ose_ioctl *ioctl = (struct Ose_ioctl *)arg;
	if (bar0_vir == 0)
		return - EFAULT;
	p = bar0_vir + ioctl->offset;
	printk(KERN_INFO "OFFSET:0x%x\n", ioctl->offset);
	printk(KERN_INFO "VAL:%d\n",ioctl->val);
	switch(cmd)
	{
		case OSE_MEM_W:
			printk(KERN_INFO "P:%p\n",p);
			*p = ioctl->val;
			printk(KERN_INFO "dbl:%x",*p);
			break;
		case OSE_MEM_R:
			ioctl->val = *p;
			printk(KERN_INFO "get dbl:%x",*p);
			printk(KERN_INFO "get dbl:%x",ioctl->val);
			break;
		default:
			break;
	}
	return 0;
}
ssize_t ose_read(struct file *file, char __user *buf,size_t length,loff_t *loff)
{
	int result;
	char * p;
	p = bar0_vir + 0x60;
	printk(KERN_INFO "OSE_READ\n");
	if ( length > DATA_LENGTH)
		length = DATA_LENGTH;

	if (bar0_vir == 0)
		return - EFAULT;
	if (copy_to_user(buf,(void *)p,length))
		result = - EFAULT;
	else{
		*loff += length;
		result += length;
	}
	printk(KERN_INFO "read %ld byte from %p",length,p);
	p = bar0_vir + 0xE0;
	if (copy_to_user(buf+length,(void *)p,length))
		result = - EFAULT;
	else{
		*loff += length;
		result += length;
	}
	printk(KERN_INFO "read %ld byte from %p",length,p);
	return result;
}
static ssize_t ose_write(struct file *file,const char __user *buf,size_t length,loff_t *loff)
{
	size_t retval = -ENOMEM;
	char * p;
	p = bar0_vir + 0xE0;
	printk(KERN_INFO "ose_write\n");
	if ( length > DATA_LENGTH)
		length = DATA_LENGTH;
	if (bar0_vir == 0)
		return - EFAULT;
	if (copy_from_user(p,buf,length))
		return -EFAULT;
	retval = length;
	return retval;
}
int  ose_release(struct inode *inode,struct file *file)
{

	return 0;
}
static struct file_operations ose_fops = {
	.owner   		=  THIS_MODULE,
	.open   		=  ose_open,
	.unlocked_ioctl =  ose_ioctl,
	.read           =  ose_read,
	.write          =  ose_write,
	.release 		=  ose_release
};
static char *pci_devnode(struct device *dev, umode_t *mode)
{
	if (!mode)
		return NULL;
	*mode = 0666;
	return NULL;
}
static int __init ipc_pci_init(void)
{
	int ret = -1;
	struct pci_dev *dev = NULL;
	dev = pci_get_device(VENDOR_ID,DEVICE_ID,NULL);
	if(dev){
		printk(KERN_INFO "get device:%x\n",dev->device);
		pci_dev_put(dev);
	}
	ret = pci_register_driver(&ipc_driver);
	DEBUG_PRINT(KERN_INFO "%s %d\n","install pci driver ",ret);
	if (ret < 0){
		DEBUG_ERR(KERN_INFO "%s\n","register failed");
		return ret;
	}

	ret=alloc_chrdev_region(&osedev.devno,0,DEVICE_NUMBER,"oseipc");
	if (ret < 0) {
		DEBUG_ERR(KERN_INFO "%s\n","register_chrdev_region failed");
		return ret;
	}

	cdev_init(&osedev.cdev, &ose_fops);
	ret = cdev_add(&osedev.cdev, osedev.devno, DEVICE_NUMBER);
	if (ret < 0) {
		printk(KERN_INFO "faield: cdev_add\n");
		return ret;
	}

	ose_class = class_create(THIS_MODULE, "ose_class");
	ose_class->devnode = pci_devnode;
	ose_dev = device_create(ose_class, NULL, osedev.devno, NULL, "ose_ipc");
	return ret;
}

static void  __exit ipc_pci_exit(void)
{
	DEBUG_PRINT(KERN_INFO "%s\n","remove pci driver");
	device_destroy(ose_class,osedev.devno);
	class_destroy(ose_class);
	cdev_del(&(osedev.cdev));
	unregister_chrdev_region(osedev.devno,DEVICE_NUMBER);
	pci_unregister_driver(&ipc_driver);
}


static int ose_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	u16 val;
	DEBUG_PRINT(KERN_INFO "ose_probe\n");
	if(pci_enable_device(dev))
	{
		DEBUG_PRINT("enable device failed.");
		return -1;
	}
	pci_read_config_word(dev,0,&val);
	DEBUG_PRINT(KERN_INFO "%x",val);
	bar0_phy = pci_resource_start(dev,0);
	if(bar0_phy<0){
		DEBUG_ERR("pci_resource_start\n");
		return -1;
	}

	bar0_length = pci_resource_len(dev,0);
	DEBUG_PRINT(KERN_INFO "bar0 length:%lx\n",bar0_length);
	if(bar0_length!=0){
		bar0_vir = (unsigned long)ioremap(bar0_phy,bar0_length);
	}

	DEBUG_PRINT(KERN_INFO "vir add:%lx\n",bar0_vir);

	return 0;
}

static void ose_remove(struct pci_dev *dev)
{

}
module_init(ipc_pci_init);

module_exit(ipc_pci_exit);

