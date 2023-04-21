#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/fs.h>
#include <linux/types.h>

#define MESSAGE_SLOT_DEVICE_MAJOR 235
#define MESSAGE_SLOT_DEVICE_NAME "message_slot"
#define MSG_SLOT_CHANNEL _IOW(MESSAGE_SLOT_DEVICE_MAJOR, 0, unsigned int)
#define BUFFER_SIZE 128

struct file_data {
    unsigned int channel_id;
};

struct channel {
    unsigned int message_size;
    char message[BUFFER_SIZE];
};

int device_open(struct inode *inode, struct file *file);
long device_ioctl(struct file *file, unsigned int cmd, unsigned long param);
ssize_t device_read(struct file *file, char __user *buffer, size_t length,
                    loff_t *offset);
ssize_t device_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *offset);
int device_release(struct inode *inode, struct file *file);

const struct file_operations fops = {.owner = THIS_MODULE,
                                     .open = device_open,
                                     .unlocked_ioctl = device_ioctl,
                                     .read = device_read,
                                     .write = device_write,
                                     .release = device_release};

#endif
