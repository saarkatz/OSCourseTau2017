#ifndef MESSAGE_SLOT_
#define MESSAGE_SLOT_

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

MODULE_LICENSE("GPL");

#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define NUM_CHANNELS 4

typedef struct Message_slot {
  char messages[NUM_CHANNELS][BUF_LEN];
} Smessage;

#endif /* !MESSAGE_SLOT_ */