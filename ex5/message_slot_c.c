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
#include <linux/slab.h>     /* for kmalloc */
#include <linux/spinlock.h> /* for spinlock */
#include <linux/ioctl.h>

#include "message_slot_c.h"
#include "message_list.h"
#include "definitions.h"

MODULE_LICENSE("GPL");

typedef struct Message_slot {
  char messages[NUM_CHANNELS][BUF_LEN];
  int channel;
  // A message slot will be restricted to a single open device
  int in_use;
  spinlock_t lock_slot;
} Smessage;

typedef struct message_slot_info {
  // Only one process is allowed to malipulate the list at a time
  spinlock_t lock_list;
} MSinfo;

static int major; /* device major number */
static Mlist *mlist; /* Messages list */
static MSinfo msinfo; /* We will restrict to one */

/***************** char device functions *********************/

/* process attempts to open the device file */
// Use spinlock to resctrict the job to single device open
static int device_open(struct inode *inode, struct file *file) {
  Smessage *device;
  unsigned long flags; // for spinlock

  PRINTK_D("%s\n", "Device open");

  // Lock the list
  spin_lock_irqsave(&msinfo.lock_list, flags);
  device = mlist_find(mlist, file->f_inode->i_ino);
  if (NULL == device) {
    device = (Smessage*)kmalloc(sizeof(*device), GFP_KERNEL);

    // As per the answers in the moodle, the buffers shall not be cleared.
    // Which is, in my opinion, bad design because the user can get data that
    // was previously in the kernel this way.
    // memset(device, 0, sizeof(*device));
    device->channel = -1;
    device->in_use = 0;
    spin_lock_init(&device->lock_slot);

    mlist_append(mlist, file->f_inode->i_ino, device);
  }
  spin_unlock_irqrestore(&msinfo.lock_list, flags);

  // Lock device
  spin_lock_irqsave(&device->lock_slot, flags);
  if (device->in_use) {
    spin_unlock_irqrestore(&device->lock_slot, flags);
    return -EBUSY;
  }
  device->in_use++;
  spin_unlock_irqrestore(&device->lock_slot, flags);

  return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
  Smessage *device;
  unsigned long flags; // for spinlock

  PRINTK_D("%s", "Device release\n");

  // Lock the list
  spin_lock_irqsave(&msinfo.lock_list, flags);
  device = mlist_find(mlist, file->f_inode->i_ino);
  spin_unlock_irqrestore(&msinfo.lock_list, flags);
  
  spin_lock_irqsave(&device->lock_slot, flags);
  device->in_use--;
  // In my opinion it is better to force the user to choose a channel whenever
  // he tries to use the device. Otherwise he will read from the last used
  // channel.
  //device->channel = -1;
  spin_unlock_irqrestore(&device->lock_slot, flags);

  return SUCCESS;
}

/* a process which has already opened
the device file attempts to read from it */
static ssize_t device_read(struct file *file, /* see include/linux/fs.h */
  char __user * buffer, /* buffer to be filled with data */
  size_t length,  /* length of the buffer */
  loff_t * offset)
{
  int i;
  int result;
  char character;
  Smessage *device;
  unsigned long flags; // for spinlock

  PRINTK_D("%s\n", "Device read");

  // Get device from list
  spin_lock_irqsave(&msinfo.lock_list, flags);
  device = mlist_find(mlist, file->f_inode->i_ino);
  spin_unlock_irqrestore(&msinfo.lock_list, flags);

  // The device must be in the list if the user had opened it
  //if (NULL == device) {
  //  return -EINVAL;
  //}

  // Make sure that a channel was set
  spin_lock_irqsave(&device->lock_slot, flags);
  if (-1 == device->channel) {
    spin_unlock_irqrestore(&device->lock_slot, flags);
    return -EINVAL;
  }
  spin_unlock_irqrestore(&device->lock_slot, flags);

  // Check that there is no fault in the user buffer
  // See https://www.fsl.cs.sunysb.edu/kernel-api/re244.html
  // and https://www.fsl.cs.sunysb.edu/kernel-api/re245.html
  // for more about get_user/put_user
  for (i = 0; i < length; i++) {
    result = get_user(character, buffer + i);
    if (0 > result) {
      return result;
    }
  }

  // At this point we can procceed to copy the message
  // device channel can't be set by ioctl to anything other than 0-BUF_LEN so
  // there wont be a check to see if channel is valid.
  for (i = 0; i < MIN(BUF_LEN, length); i++) {
    put_user(device->messages[device->channel][i], buffer + i);
  }

  return i;
}

static ssize_t device_write(struct file *file,
  const char __user * buffer, size_t length, loff_t * offset) {
  int i;
  int byte_count = 0;
  int result;
  Smessage *device;
  char character;
  unsigned long flags; // for spinlock

  PRINTK_D("%s\n", "Device write");

  // Get device from list
  spin_lock_irqsave(&msinfo.lock_list, flags);
  device = mlist_find(mlist, file->f_inode->i_ino);
  spin_unlock_irqrestore(&msinfo.lock_list, flags);

  // The device must be in the list if the user had opened it
  //if (NULL == device) {
  //  return -EINVAL;
  //}

  // Make sure that a channel was set
  spin_lock_irqsave(&device->lock_slot, flags);
  if (-1 == device->channel) {
    spin_unlock_irqrestore(&device->lock_slot, flags);
    return -EINVAL;
  }
  spin_unlock_irqrestore(&device->lock_slot, flags);

  // Check that there is no fault in the user buffer
  for (i = 0; i < length; i++) {
    result = get_user(character, buffer + i);
    if (0 > result) {
      return result;
    }
  }

  // At this point we can procceed to copy the message
  // device channel can't be set by ioctl to anything other than 0-BUF_LEN so
  // there wont be a check to see if channel is valid.
  byte_count = i;
  for (i = 0; i < BUF_LEN; i++) {
    if (i < byte_count) {
      get_user(device->messages[device->channel][i], buffer + i);
    }
    else {
      device->messages[device->channel][i] = 0;
    }
  }


  return byte_count;
}

//----------------------------------------------------------------------------
static long device_ioctl( //struct inode*  inode,
  struct file*   file,
  unsigned int   ioctl_num,/* The number of the ioctl */
  unsigned long  ioctl_param) /* The parameter to it */ {
  Smessage *device;
  unsigned long flags; // for spinlock

  /* Switch according to the ioctl called */
  switch (ioctl_num) {
  case IOCTL_SET_CHAN:
    PRINTK_D("Changing channel of deivce %lu to %lu\n", file->f_inode->i_ino,
      ioctl_param);
    if (0 <= ioctl_param && ioctl_param < NUM_CHANNELS) {
      // Lock the list
      spin_lock_irqsave(&msinfo.lock_list, flags);
      device = mlist_find(mlist, file->f_inode->i_ino);
      spin_unlock_irqrestore(&msinfo.lock_list, flags);

      spin_lock_irqsave(&device->lock_slot, flags);
      device->channel = ioctl_param;
      spin_unlock_irqrestore(&device->lock_slot, flags);
    }
    else {
      return -EINVAL;
    }
    break;
  default:
    PRINTK_D("%s\n", "Ioctl - unknown message");
    return -EINVAL;
    break;
  }

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
  .unlocked_ioctl = device_ioctl,
};

/* Called when module is loaded.
* Initialize the module - Register the character device */
static int __init simple_init(void) {
  // Initialize list lock
  memset(&msinfo, 0, sizeof(msinfo));
  spin_lock_init(&msinfo.lock_list);

  // Initialize message list
  mlist = mlist_create();
  if (NULL == mlist) {
    PRINTK_I("%s", ERR_CREATE_MLIST);
    return -ENOMEM;
  }

  /* Register a character device. Get newly assigned major num */
  // If MAGIC_NUM > 0 register_chrdev returns 0 on success.
  // See https://www.fsl.cs.sunysb.edu/kernel-api/re941.html
  major = register_chrdev(MAGIC_NUM, DEVICE_RANGE_NAME, &Fops /* our own file operations struct */);
  /* Negative values signify an error */
  if (major < 0) {
    PRINTK_I(KERN_ALERT "%s failed with %d\n",
      "Sorry, registering the character device ", MAGIC_NUM);
    mlist_destroy(mlist);
    return major;
  }

  PRINTK_D("Module registered. The major device number is %d.\n", MAGIC_NUM);
  return SUCCESS;
}

/* Cleanup - unregister the appropriate file from /proc */
// Unloading the module while a process is using it can crush the module
// or atleast make it derefrece freed memory, which is bad.
static void __exit simple_cleanup(void) {
  PRINTK_D("%s", "Unloading module\n");

  // Free message list
  mlist_destroy(mlist);

  unregister_chrdev(MAGIC_NUM, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);
