#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/radix-tree.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include "message_slot.h"

struct file_data {
    unsigned int channel_id;
    unsigned int minor;
};

struct channel {
    unsigned int message_size;
    char message[BUFFER_SIZE];
};

/* An array of radix trees, which holds a tree for each minor.
 Each tree is a mapping between the channel id and its matching channel struct.
 */
static struct radix_tree_root *channels_trees[MINORS_COUNT];

static struct channel *get_channel_from_file(struct file *file) {
    unsigned int minor = iminor(file->f_inode);
    struct radix_tree_root *channels_tree = channels_trees[minor];
    struct file_data *fdata = file->private_data;
    return radix_tree_lookup(channels_tree, fdata->channel_id);
}

static void free_radix_tree(struct radix_tree_root *root) {
    struct radix_tree_iter iter;
    void **slot;

    if (root != NULL) {
        radix_tree_for_each_slot(slot, root, &iter, 0) {
            kfree(*slot);
        }
    }
}

int device_open(struct inode *inode, struct file *file) {
    struct file_data *fdata;
    if ((fdata = kmalloc(sizeof(struct file_data), GFP_KERNEL)) == NULL) {
        printk(KERN_ERR "Couldn't allocate message slot file descriptor");
        return 1;
    }

    fdata->channel_id = 0;
    fdata->minor = iminor(inode);
    file->private_data = fdata;

    if (channels_trees[fdata->minor] == NULL) {
        channels_trees[fdata->minor] =
            kmalloc(sizeof(struct radix_tree_root), GFP_KERNEL);
        if (channels_trees[fdata->minor] == NULL) {
            printk(KERN_ERR "Couldn't allocate radix tree for message slot");
            return 1;
        }

        INIT_RADIX_TREE(channels_trees[fdata->minor], GFP_KERNEL);
    }

    printk(KERN_INFO "Opened message slot file");
    return 0;
}

long device_ioctl(struct file *file, unsigned int cmd, unsigned long param) {
    struct file_data *fdata = (struct file_data *)file->private_data;
    struct radix_tree_root *channels_tree;
    struct channel *channel;

    if (cmd != MSG_SLOT_CHANNEL || param == 0) {
        return -EINVAL;
    }

    fdata->channel_id = (unsigned int)param;

    channels_tree = channels_trees[fdata->minor];
    if (radix_tree_lookup(channels_tree, fdata->channel_id) == NULL) {
        channel = kmalloc(sizeof(struct channel), GFP_KERNEL);
        radix_tree_insert(channels_tree, fdata->channel_id, channel);
    }

    return 0;
}

ssize_t device_read(struct file *file, char __user *buffer, size_t length,
                    loff_t *offset) {
    struct file_data *fdata = (struct file_data *)file->private_data;
    struct channel *channel;

    if (fdata->channel_id == 0) {
        return -EINVAL;
    }
    if (length == 0 || length > BUFFER_SIZE) {
        return -EMSGSIZE;
    }

    channel = get_channel_from_file(file);
    if (channel->message_size == 0) {
        return -EWOULDBLOCK;
    }
    if (length < channel->message_size) {
        return -ENOSPC;
    }

    if (copy_to_user(buffer, channel->message, channel->message_size) > 0) {
        return -EFAULT;
    }

    return channel->message_size;
}

ssize_t device_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *offset) {
    struct file_data *fdata = file->private_data;
    struct channel *channel;
    if (fdata->channel_id == 0) {
        return -EINVAL;
    }
    if (length == 0 || length > BUFFER_SIZE) {
        return -EMSGSIZE;
    }

    channel = get_channel_from_file(file);
    if (copy_from_user(channel->message, buffer, length) > 0) {
        return -EFAULT;
    }
    channel->message_size = length;
    return length;
}

int device_release(struct inode *inode, struct file *file) {
    kfree(file->private_data);
    return 0;
}

static const struct file_operations fops = {.owner = THIS_MODULE,
                                            .open = device_open,
                                            .unlocked_ioctl = device_ioctl,
                                            .read = device_read,
                                            .write = device_write,
                                            .release = device_release};

int init_module(void) {
    if (register_chrdev(MESSAGE_SLOT_DEVICE_MAJOR, MESSAGE_SLOT_DEVICE_NAME,
                        &fops) < 0) {
        printk(KERN_ERR "Error registering message_slot\n");
        return 1;
    }

    printk(KERN_INFO "Registered message slot\n");
    return 0;
}

void cleanup_module(void) {
    size_t i;
    unregister_chrdev(MESSAGE_SLOT_DEVICE_MAJOR, MESSAGE_SLOT_DEVICE_NAME);
    printk(KERN_INFO "Unregistered message slot\n");

    for (i = 0; i < MINORS_COUNT; i++) {
        free_radix_tree(channels_trees[i]);
    }
}
