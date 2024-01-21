#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MESSAGE_SLOT_DEVICE_MAJOR 235
#define MESSAGE_SLOT_DEVICE_NAME "message_slot"
#define MSG_SLOT_CHANNEL _IOW(MESSAGE_SLOT_DEVICE_MAJOR, 0, unsigned int)
#define BUFFER_SIZE 128
#define MINORS_COUNT 256

#endif
