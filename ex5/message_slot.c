#include "message_slot.h"
#include "message_list.h"
#include "definitions.h"

static int major; /* device major number */

/***************** char device functions *********************/

/* process attempts to open the device file */
// Use spinlock to resctrict the job to single device open
static int device_open(struct inode *inode, struct file *file) {
  PRINTK_I("Device open\n");
  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
  PRINTK_I("Device release\n");
  return SUCCESS;
}

/* a process which has already opened
the device file attempts to read from it */
static ssize_t device_read(struct file *file, /* see include/linux/fs.h */
  char __user * buffer, /* buffer to be filled with data */
  size_t length,  /* length of the buffer */
  loff_t * offset)
{
  PRINTK_I("device_read(%p,%d)\n", file, length);

  return -EINVAL; // invalid argument error
}

static ssize_t device_write(struct file *file,
  const char __user * buffer, size_t length, loff_t * offset) {
  PRINTK_I("Device write\n");
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
    PRINTK_I(KERN_ALERT "%s failed with %d\n",
      "Sorry, registering the character device ", major);
    return major;
  }

  PRINTK_I("Module registered. The major device number is %d.\n", major);
  return 0;
}

/* Cleanup - unregister the appropriate file from /proc */
static void __exit simple_cleanup(void) {
  PRINTK_I("Unloading module\n");
  unregister_chrdev(major, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);
