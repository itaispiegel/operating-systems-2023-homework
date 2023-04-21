#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "message_slot.h"

int device_open(struct inode *inode, struct file *file) {
    struct file_data *fdata =
        (struct file_data *)kmalloc(sizeof(struct file_data), GFP_KERNEL);
    if (fdata == NULL) {
        return 1;
    }
    fdata->channel_id = 0;
    file->private_data = fdata;
    return 0;
}

long device_ioctl(struct file *file, unsigned int cmd, unsigned long param) {
    if (cmd != MSG_SLOT_CHANNEL || param == 0) {
        return -EINVAL;
    }

    struct file_data *fdata = file->private_data;
    fdata->channel_id = (unsigned int)param;
    return 0;
}

ssize_t device_read(struct file *file, char __user *buffer, size_t length,
                    loff_t *offset) {
    struct file_data *fdata = file->private_data;
    if (fdata->channel_id == 0) {
        return -EINVAL;
    }
    if (length == 0 || length > BUFFER_SIZE) {
        return -EMSGSIZE;
    }
    return 0;
}
ssize_t device_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *offset) {
    struct file_data *fdata = file->private_data;
    if (fdata->channel_id == 0) {
        return -EINVAL;
    }
    return 0;
}

int device_release(struct inode *inode, struct file *file) {
    kfree(file->private_data);
    return 0;
}

int init_module(void) {
    printk(KERN_INFO "Registering message slot\n");
    if (register_chrdev(MESSAGE_SLOT_DEVICE_MAJOR, MESSAGE_SLOT_DEVICE_NAME,
                        &fops) < 0) {
        printk(KERN_ERR "Error registering message_slot\n");
    }
    return 0;
}

void cleanup_module(void) {
    printk(KERN_INFO "Unregistering message slot\n");
    unregister_chrdev(MESSAGE_SLOT_DEVICE_MAJOR, MESSAGE_SLOT_DEVICE_NAME);
}
