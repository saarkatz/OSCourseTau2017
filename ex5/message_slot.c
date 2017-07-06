/* Declare what kind of code we want from the header files
Defining __KERNEL__ and MODULE allows us to access kernel-level
code not usually available to userspace programs. */
#undef __KERNEL__
#define __KERNEL__ /* We're part of the kernel */
#undef MODULE
#define MODULE     /* Not a permanent part, though. */

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <asm/uaccess.h>    /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/

#include "message_slot.h"

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define NUM_CHANNELS 4

typedef struct Message_slot {
  char messages[NUM_CHANNELS][BUF_LEN];
} message_slot;

static int major; /* device major number */


/***************** char device functions *********************/

/* process attempts to open the device file */
static int device_open(struct inode *inode, struct file *file) {
  printk("Device open\n");
  printk("device_open(%p)\n", file);
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
  printk("Device release\n");
  return SUCCESS;
}

/* a process which has already opened
the device file attempts to read from it */
static ssize_t device_read(struct file *file, /* see include/linux/fs.h */
  char __user * buffer, /* buffer to be filled with data */
  size_t length,  /* length of the buffer */
  loff_t * offset)
{
  printk("device_read(%p,%d) - operation not supported yet\n", file, length);

  return -EINVAL; // invalid argument error
}

static ssize_t device_write(struct file *file,
  const char __user * buffer, size_t length, loff_t * offset) {
  printk("Device write\n");
  return SUCCESS;
}


/************** Module Declarations *****************/

/* This structure will hold the functions to be called
* when a process does something to the device we created */
struct file_operations Fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release,    /* a.k.a. close */
};

/* Called when module is loaded.
* Initialize the module - Register the character device */
static int __init simple_init(void) {
  /* Register a character device. Get newly assigned major num */
  major = register_chrdev(0, DEVICE_RANGE_NAME, &Fops /* our own file operations struct */);

  /* Negative values signify an error */
  if (major < 0) {
    printk(KERN_ALERT "%s failed with %d\n",
      "Sorry, registering the character device ", major);
    return major;
  }

  printk("Module registered. The major device number is %d.\n", major);
  return 0;
}

/* Cleanup - unregister the appropriate file from /proc */
static void __exit simple_cleanup(void) {
  printk("Unloading module");
  unregister_chrdev(major, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);
