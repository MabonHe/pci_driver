
#include<stdio.h>
#include<string.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ose_pci.h>
#include <stdlib.h>
#define MAX_LENAGTH 124

int readmsg();
int writemsgtofw(unsigned int doorbell,int length, void *msg);
int command(int argc, char **argv);
int cleandbl();
int cleandbl()
{
     struct Ose_ioctl *ose_io = malloc(sizeof(struct Ose_ioctl));
    ose_io->offset = 0x54;
    ose_io->val = 1;
    char buf[256];
    unsigned int host2ish_doorbell, ish2host_doorbell;
    memset(buf,0,sizeof(buf));
    int fd = open("/dev/ose_ipc",O_RDWR);
    if (fd < 0)
    {
        printf("open dev failed\n");
        return -1;
    }
    int ret = ioctl(fd,OSE_MEM_R,ose_io);
    if (ret < 0)
    {
        printf("ret:%d\n",ret);
        return -1;
    }
    printf("dbl val:0x%x\n",ose_io->val);
    ish2host_doorbell = ose_io->val;
    ose_io->val = 0x80000000;
    if (ose_io->val & 0x80000000)
    {
        ret = read(fd,buf,MAX_LENAGTH);
        ose_io->val = 0;
        printf("clean doorbell\n");
        ret = ioctl(fd,OSE_MEM_W,ose_io);
    }
    ose_io->offset = 0x48;
    ret = ioctl(fd,OSE_MEM_R,ose_io);
    host2ish_doorbell = ose_io->val;
    printf("hosttoish:0x%x\n",host2ish_doorbell);
    for(int i = 1;i <= MAX_LENAGTH;i++)
    {
        printf("0x%02x",buf[i-1]);
        printf(" ");
        if ((i%10) == 0)
            printf("\n");

    }
    printf("\n");
    int offset = 123;
    printf("ishtohost:0x%x\n",ish2host_doorbell);
    for(int i = 1;i <= MAX_LENAGTH;i++)
    {
       printf("0x%02x",buf[(i-1)+offset]);
        printf(" ");
        if ((i%10) == 0 )
            printf("\n");
    }
    printf("\n");
    close(fd);
    free(ose_io);
    return 0;
}
void usage()
{
    printf("oseAPP help\n");

}
int command(int argc, char **argv)
{

    if (strcmp("r",argv[1]) == 0)
    {
        readmsg();
    }else if (strcmp("w",argv[1]) == 0)
    {
        int msglength = 0,length;
        unsigned int msg[32],doorbell = 0x80000000;
        doorbell = strtoul(argv[2],NULL,16);
        printf("doorbell:0x%x\n",doorbell);
        for(int i = 3; i < argc && msglength < 32; i++)
        {
            msg[msglength] = strtoul(argv[i],NULL,16);
            printf("msg:%x\n",msg[msglength]);
            msglength++;
        }
        length = msglength *sizeof(unsigned int);
        printf("writelength:%d\n",length);
        printf("msglength:%d\n",msglength);
        writemsgtofw(doorbell,length,msg);
    }else if (0 == strcmp("help",argv[1]))
    {
        usage();
    }else if (strcmp("clean",argv[1]) == 0)
    {
        cleandbl();
    }
    else
    {
        printf("nothing to do\n");
    }

    return 0;
}
int writemsgtofw(unsigned int doorbell,int length, void *msg)
{
    struct Ose_ioctl *ose_io = malloc(sizeof(struct Ose_ioctl));
    ose_io->offset = 0x48;
    ose_io->val = 1;
    char buf[256];
    memset(buf,0,sizeof(buf));
    int fd = open("/dev/ose_ipc",O_RDWR);
    if (fd < 0)
    {
        printf("open dev failed\n");
        return -1;
    }
    int ret = ioctl(fd,OSE_MEM_R,ose_io);
    if (ret < 0)
    {
        printf("ret:%d\n",ret);
        return -1;
    }
    printf("get:doorbell:0x%x\n",ose_io->val);
    ose_io->val = doorbell | ose_io->val;
    printf("set:doorbell:0x%x\n",ose_io->val);
    ret = ioctl(fd,OSE_MEM_W,ose_io);
    if (ret < 0)
    {
        printf("ret:%d\n",ret);
        return -1;
    }
    printf("set doorbell successfully\n");
    ret = write(fd,msg,length);
    printf("writelength:%d\n",ret);
    if (ret < 0)
    {
        printf("write msg failed\n");
        return -1;
    }
    close(fd);
    free(ose_io);
    return 0;
}
int readmsg()
{
    struct Ose_ioctl *ose_io = malloc(sizeof(struct Ose_ioctl));
    ose_io->offset = 0x54;
    ose_io->val = 1;
    char buf[256];
    unsigned int host2ish_doorbell, ish2host_doorbell;
    memset(buf,0,sizeof(buf));
    int fd = open("/dev/ose_ipc",O_RDWR);
    if (fd < 0)
    {
        printf("open dev failed\n");
        return -1;
    }
    int ret = ioctl(fd,OSE_MEM_R,ose_io);
    if (ret < 0)
    {
        printf("ret:%d\n",ret);
        return -1;
    }
    printf("dbl val:0x%x\n",ose_io->val);
    ish2host_doorbell = ose_io->val;

    ret = read(fd,buf,MAX_LENAGTH);
    printf("ishtohost:0x%x\n",ish2host_doorbell);
    ose_io->offset = 0x48;
    ret = ioctl(fd,OSE_MEM_R,ose_io);
    host2ish_doorbell = ose_io->val;
    printf("hosttoish:0x%x\n",host2ish_doorbell);
    for(int i = 1;i <= MAX_LENAGTH;i++)
    {
        printf("0x%02x",buf[i-1]);
        printf(" ");
        if ((i%10) == 0)
            printf("\n");
    }
    printf("\n");
    int offset = 124;
    printf("hosttoish:0x%x\n",host2ish_doorbell);
    for(int i = 1;i <= MAX_LENAGTH;i++)
    {
       printf("0x%02x",buf[(i-1)+offset]);
        printf(" ");
        if ((i%10) == 0 )
            printf("\n");
    }
    printf("\n");
    close(fd);
    free(ose_io);
    return 0;
}
int main(int argc, char **argv)
{

    if (argc < 2)
    {
        usage();
        return -1;
    }
    command(argc,argv);
    return 0;
}

