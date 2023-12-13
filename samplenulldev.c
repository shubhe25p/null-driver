#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/sched.h>

#define MY_MAJOR 420
#define MY_MINOR 0
#define DEVICE_NAME "samplenulldev"

int major, minor;
char *kernel_buffer;

struct cdev *my_cdev;
int actual_rx_size=0;

MODULE_AUTHOR("Shubh Pachchigar");
MODULE_DESCRIPTION("Sample Null Device Driver");
MODULE_LICENSE("GPL");

struct myds {
    int count;
} myds;

static ssize_t myWrite(struct file *fs, const char __user *buf, size_t hsize, loff_t *off){
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    ds->count += 1;
    printk(KERN_INFO "We wrote: %lu on write number %d\n", hsize, ds->count);
    return hsize;
}

static ssize_t myRead(struct file *fs, char __user *buf, size_t hsize, loff_t *off){
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    printk(KERN_INFO "We read: %lu on read number %d\n", hsize, ds->count);
    return 0;
}

static int myOpen(struct inode *inode, struct file *fs)
{
    struct myds * ds;
    ds = vmalloc(sizeof(struct myds));
    if(ds == 0){
        printk(KERN_ERR "myOpen: vmalloc failed\n");
        return -1;
    }
    ds->count = 0;
    fs->private_data = ds;
    return 0;
}

static int myClose(struct inode *inode, struct file *fs)
{
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    vfree(ds);
    return 0;
}

static long myIoctl(struct file *fs, unsigned int cmd, unsigned int data)
{
    int * count;
    struct myds * ds;
    ds = (struct myds *) fs->private_data;
    if(cmd != 3){
        printk(KERN_ERR "myIoctl: invalid command\n");
        return -1;
    }
    count = (int *) data;
    *count = ds->count;
    return 0;
}

struct file_operations fops = {
    .open = myOpen,
    .release = myClose,
    .write = myWrite,
    .read = myRead,
    .unlocked_ioctl = myIoctl,
    .owner = THIS_MODULE
}

int init_module(void){
    int result, registers;
    dev_t devno;

    devno = MKDEV(MY_MAJOR, MY_MINOR);

    registers = register_chrdev_region(devno, 1, DEVICE_NAME);
    printk(KERN_INFO "Register chrdev returned %d\n", registers);
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    result = cdev_add(&my_cdev, devno, 1);
    printk(KERN_INFO "cdev_add returned %d\n", result);
    printk(KERN_INFO "NULL device driver registered.\n");

    if(result < 0){
        printk(KERN_ERR "Register chardev failed with %d\n", result);
    }
    return result;
}

void cleanup_module(void){
    dev_t devno = MKDEV(MY_MAJOR, MY_MINOR);
    unregister_chrdev_region(devno, 1);
    cdev_del(&my_cdev);
    printk(KERN_INFO "NULL device driver unregistered.\n");
}
