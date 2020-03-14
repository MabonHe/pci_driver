/*************************************************************************
	> File Name: pcie.h
	> Author: 
	> Mail: 
	> Created Time: 2018年09月28日 星期五 21时18分08秒
 ************************************************************************/

#ifndef __OSE_PCI_H__
#define __OSE_PCI_H__ 
#include<linux/ioctl.h>
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
#define OSE_MEM_WR _IOWR(OSE_MAGIC,2,struct Ose_ioctl)
#endif
