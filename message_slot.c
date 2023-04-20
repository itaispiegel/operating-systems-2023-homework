#include <linux/kernel.h>
#include <linux/module.h>

#include "message_slot.h"

int init_module(void) {
    printk(KERN_INFO "Hello, World!\n");
    return 0;
}

void cleanup_module(void) { printk(KERN_INFO "Goodbye world\n"); }
