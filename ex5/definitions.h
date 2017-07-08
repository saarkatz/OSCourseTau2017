#ifndef DEFINITIONS_
#define DEFINITIONS_

/* Declare what kind of code we want from the header files
Defining __KERNEL__ and MODULE allows us to access kernel-level
code not usually available to userspace programs. */
#undef __KERNEL__
#define __KERNEL__ /* We're part of the kernel */
#undef MODULE
#define MODULE     /* Not a permanent part, though. */

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */

// Debug flags
#define DEBUG 1

// Numeral constants
#define SUCCESS 0

// Error message definitions
#define ERR_CREATE_MLIST "Error creating message list"
#define ERR_CREATE_MNODE "Error creating message node"

// Print macros
#define PRINTK_I(...) do { printk(__VA_ARGS__); } while(0)

// When using PRINTK_D always use a format string because in c the 
// "format, ..." macro will always add a comma after the format variable,
// which will cause a compiler error.
#if DEBUG == 0
#define PRINTK_D(...) 
#else
#define PRINTK_D(format, ...) do {\
printk("%-30.30s - %-4d: " format, __FILE__, __LINE__, __VA_ARGS__); } while (0)
#endif

#endif /* !DEFINITIONS_ */