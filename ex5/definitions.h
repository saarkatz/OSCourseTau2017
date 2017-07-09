#ifndef DEFINITIONS_
#define DEFINITIONS_

// Debug flags
#define DEBUG 1

// Numeral constants
#define SUCCESS 0

// Error message definitions
#define ERR_CREATE_MLIST "Error creating message list"
#define ERR_CREATE_MNODE "Error creating message node"

// Print macros
#define PRINTK_I(format, ...) do { printk(format, __VA_ARGS__); } while(0)

// When using PRINTK_D always use a format string because in c the 
// "format, ..." macro will always add a comma after the format variable,
// which will cause a compiler error.
#if DEBUG == 0
#define PRINTK_D(...) 
#else
#define PRINTK_D(format, ...) do {\
printk("%-30.30s - %-4d: " format, __FILE__, __LINE__, __VA_ARGS__); } while (0)
#endif

// Helper macros
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif /* !DEFINITIONS_ */